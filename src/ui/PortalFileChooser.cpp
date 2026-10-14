#include "PortalFileChooser.h"

#include <systemd/sd-bus.h>

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <unistd.h>

namespace {

struct BusMessage {
  sd_bus_message *message = nullptr;
  ~BusMessage() {
    if (message)
      sd_bus_message_unref(message);
  }
};

struct BusError {
  sd_bus_error error = SD_BUS_ERROR_NULL;
  ~BusError() { sd_bus_error_free(&error); }
};

std::string ToLower(std::string value) {
  for (char &c : value)
    c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
  return value;
}

std::string NormalizeExtension(std::string extension) {
  if (extension.empty())
    return extension;
  if (extension.front() != '.')
    extension.insert(extension.begin(), '.');
  return ToLower(extension);
}

bool IsImageExtension(const std::string &extension) {
  static const char *kImage[] = {".png", ".jpg", ".jpeg", ".webp", ".bmp",
                                 ".tga", ".gif", ".hdr",  ".psd",  ".pic",
                                 ".ppm", ".pgm", ".pnm"};
  const std::string lowered = NormalizeExtension(extension);
  for (const char *candidate : kImage) {
    if (lowered == candidate)
      return true;
  }
  return false;
}

std::string PercentDecode(const std::string &input) {
  std::string out;
  out.reserve(input.size());
  auto hex = [](char c) -> int {
    if (c >= '0' && c <= '9')
      return c - '0';
    if (c >= 'a' && c <= 'f')
      return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
      return c - 'A' + 10;
    return -1;
  };

  for (size_t i = 0; i < input.size(); ++i) {
    if (input[i] == '%' && i + 2 < input.size()) {
      const int hi = hex(input[i + 1]);
      const int lo = hex(input[i + 2]);
      if (hi >= 0 && lo >= 0) {
        out.push_back(static_cast<char>((hi << 4) | lo));
        i += 2;
        continue;
      }
    }
    out.push_back(input[i]);
  }
  return out;
}

std::filesystem::path UriToPath(const char *uri) {
  if (!uri)
    return {};

  std::string value = uri;
  constexpr const char *kFile = "file://";
  if (value.rfind(kFile, 0) == 0)
    value = value.substr(std::strlen(kFile));
  else if (value.rfind("file:", 0) == 0)
    value = value.substr(5);

  if (!value.empty() && value.front() != '/') {
    const auto slash = value.find('/');
    if (slash != std::string::npos)
      value = value.substr(slash);
  }

  return std::filesystem::path(PercentDecode(value));
}

std::string SenderToPathElement(const char *uniqueName) {
  std::string sender = uniqueName ? uniqueName : "";
  if (!sender.empty() && sender.front() == ':')
    sender.erase(sender.begin());
  for (char &c : sender) {
    if (c == '.')
      c = '_';
  }
  return sender;
}

std::string MakeHandleToken(unsigned int counter) {
  std::ostringstream out;
  out << "pigeon" << getpid() << "n" << counter;
  return out.str();
}

std::filesystem::path ExistingDirectory(const std::filesystem::path &path) {
  std::error_code error;
  if (path.empty())
    return {};
  if (std::filesystem::is_directory(path, error))
    return path;
  if (std::filesystem::is_regular_file(path, error))
    return path.parent_path();
  return {};
}

int AppendFilterRule(sd_bus_message *message, uint32_t kind,
                     const char *value) {
  return sd_bus_message_append(message, "(us)", kind, value);
}

int AppendNamedFilter(sd_bus_message *message, const char *name,
                      const std::vector<std::pair<uint32_t, const char *>> &rules) {
  int r = sd_bus_message_open_container(message, 'r', "sa(us)");
  if (r < 0)
    return r;
  r = sd_bus_message_append(message, "s", name);
  if (r < 0)
    return r;
  r = sd_bus_message_open_container(message, 'a', "(us)");
  if (r < 0)
    return r;
  for (const auto &rule : rules) {
    r = AppendFilterRule(message, rule.first, rule.second);
    if (r < 0)
      return r;
  }
  r = sd_bus_message_close_container(message);
  if (r < 0)
    return r;
  return sd_bus_message_close_container(message);
}

} // namespace

PortalFileChooser &PortalFileChooser::Instance() {
  static PortalFileChooser chooser;
  return chooser;
}

const std::vector<std::string> &PortalFileChooser::ImageExtensions() {
  static const std::vector<std::string> extensions = {
      ".png", ".jpg", ".jpeg", ".webp", ".bmp", ".tga",
      ".gif", ".hdr", ".psd",  ".pic",  ".ppm", ".pgm"};
  return extensions;
}

const std::vector<std::string> &PortalFileChooser::ModelExtensions() {
  static const std::vector<std::string> extensions = {".obj", ".glb", ".gltf",
                                                      ".fbx"};
  return extensions;
}

const std::vector<std::string> &PortalFileChooser::SceneExtensions() {
  static const std::vector<std::string> extensions = {".json"};
  return extensions;
}

PortalFileChooser::~PortalFileChooser() {
  ResetRequest();
  if (bus_)
    sd_bus_unref(bus_);
}

bool PortalFileChooser::IsBusy() const { return busy_; }

const std::filesystem::path &PortalFileChooser::LastDirectory() const {
  return lastDirectory_;
}

void PortalFileChooser::ResetRequest() {
  if (matchSlot_)
    matchSlot_ = sd_bus_slot_unref(matchSlot_);
  unrefMatchAfterCallback_ = false;
  busy_ = false;
}

bool PortalFileChooser::EnsureBus() {
  if (bus_)
    return true;

  const int r = sd_bus_open_user(&bus_);
  if (r < 0) {
    std::cerr << "PortalFileChooser: failed to connect to the session bus: "
              << strerror(-r) << std::endl;
    bus_ = nullptr;
    return false;
  }
  return true;
}

void PortalFileChooser::Poll() {
  if (!bus_)
    return;

  for (;;) {
    const int r = sd_bus_process(bus_, nullptr);
    if (r < 0) {
      std::cerr << "PortalFileChooser: sd_bus_process failed: " << strerror(-r)
                << std::endl;
      ResetRequest();
      cancelled_ = true;
      completed_ = true;
      selected_.clear();
      return;
    }
    if (r == 0)
      break;
  }

  if (unrefMatchAfterCallback_) {
    unrefMatchAfterCallback_ = false;
    if (matchSlot_)
      matchSlot_ = sd_bus_slot_unref(matchSlot_);
  }
}

bool PortalFileChooser::ConsumeCompletion(std::filesystem::path &outPath,
                                          bool &cancelled) {
  if (!completed_)
    return false;

  completed_ = false;
  cancelled = cancelled_ || selected_.empty();
  cancelled_ = false;
  outPath = std::move(selected_);
  selected_.clear();
  if (cancelled)
    outPath.clear();
  return true;
}

bool PortalFileChooser::SubscribeToResponse(const std::string &objectPath) {
  if (matchSlot_)
    matchSlot_ = sd_bus_slot_unref(matchSlot_);

  const std::string match =
      std::string("type='signal',"
                  "sender='org.freedesktop.portal.Desktop',"
                  "interface='org.freedesktop.portal.Request',"
                  "member='Response',"
                  "path='") +
      objectPath + "'";

  const int r =
      sd_bus_add_match(bus_, &matchSlot_, match.c_str(), OnResponse, this);
  if (r < 0) {
    std::cerr << "PortalFileChooser: failed to subscribe to Response: "
              << strerror(-r) << std::endl;
    matchSlot_ = nullptr;
    return false;
  }
  return true;
}

int PortalFileChooser::OnResponse(sd_bus_message *message, void *userdata,
                                  sd_bus_error *) {
  auto *self = static_cast<PortalFileChooser *>(userdata);
  if (!self)
    return 0;

  uint32_t response = 1;
  int r = sd_bus_message_read(message, "u", &response);
  if (r < 0) {
    self->cancelled_ = true;
    self->completed_ = true;
    self->selected_.clear();
    self->busy_ = false;
    self->unrefMatchAfterCallback_ = true;
    return 0;
  }

  if (response != 0) {
    self->cancelled_ = true;
    self->completed_ = true;
    self->selected_.clear();
    self->busy_ = false;
    self->unrefMatchAfterCallback_ = true;
    return 0;
  }

  r = sd_bus_message_enter_container(message, 'a', "{sv}");
  if (r < 0) {
    self->cancelled_ = true;
    self->completed_ = true;
    self->busy_ = false;
    self->unrefMatchAfterCallback_ = true;
    return 0;
  }

  std::filesystem::path chosen;
  while (sd_bus_message_enter_container(message, 'e', "sv") > 0) {
    const char *key = nullptr;
    sd_bus_message_read(message, "s", &key);
    if (key && std::strcmp(key, "uris") == 0) {
      sd_bus_message_enter_container(message, 'v', "as");
      sd_bus_message_enter_container(message, 'a', "s");
      const char *uri = nullptr;
      if (sd_bus_message_read(message, "s", &uri) > 0)
        chosen = UriToPath(uri);
      while (sd_bus_message_skip(message, "s") > 0) {
      }
      sd_bus_message_exit_container(message);
      sd_bus_message_exit_container(message);
    } else {
      sd_bus_message_skip(message, "v");
    }
    sd_bus_message_exit_container(message);
  }
  sd_bus_message_exit_container(message);

  if (chosen.empty()) {
    self->cancelled_ = true;
    self->selected_.clear();
  } else {
    self->cancelled_ = false;
    self->selected_ = chosen;
    const std::filesystem::path parent = chosen.parent_path();
    if (!parent.empty())
      self->lastDirectory_ = parent;
  }

  self->completed_ = true;
  self->busy_ = false;
  self->unrefMatchAfterCallback_ = true;
  return 0;
}

bool PortalFileChooser::CallOpenFile(const std::string &title,
                                     const std::vector<std::string> &extensions,
                                     const std::filesystem::path &folder,
                                     const std::string &handleToken,
                                     std::string &outHandle) {
  BusMessage call;
  int r = sd_bus_message_new_method_call(
      bus_, &call.message, "org.freedesktop.portal.Desktop",
      "/org/freedesktop/portal/desktop", "org.freedesktop.portal.FileChooser",
      "OpenFile");
  if (r < 0)
    return false;

  r = sd_bus_message_append(call.message, "ss", "", title.c_str());
  if (r < 0)
    return false;

  r = sd_bus_message_open_container(call.message, 'a', "{sv}");
  if (r < 0)
    return false;

  r = sd_bus_message_append(call.message, "{sv}", "handle_token", "s",
                            handleToken.c_str());
  if (r < 0)
    return false;
  r = sd_bus_message_append(call.message, "{sv}", "modal", "b", 0);
  if (r < 0)
    return false;
  r = sd_bus_message_append(call.message, "{sv}", "multiple", "b", 0);
  if (r < 0)
    return false;

  if (!folder.empty()) {
    const std::string folderStr = folder.string();
    r = sd_bus_message_open_container(call.message, 'e', "sv");
    if (r < 0)
      return false;
    r = sd_bus_message_append(call.message, "s", "current_folder");
    if (r < 0)
      return false;
    r = sd_bus_message_open_container(call.message, 'v', "ay");
    if (r < 0)
      return false;
    r = sd_bus_message_append_array(call.message, 'y', folderStr.c_str(),
                                    folderStr.size() + 1);
    if (r < 0)
      return false;
    r = sd_bus_message_close_container(call.message);
    if (r < 0)
      return false;
    r = sd_bus_message_close_container(call.message);
    if (r < 0)
      return false;
  }

  std::vector<std::string> globs;
  bool wantsImages = false;
  bool wantsModels = false;
  bool wantsScenes = false;
  for (const std::string &extension : extensions) {
    const std::string normalized = NormalizeExtension(extension);
    if (normalized.empty())
      continue;
    globs.push_back("*" + normalized);
    if (IsImageExtension(normalized))
      wantsImages = true;
    if (normalized == ".obj" || normalized == ".glb" || normalized == ".gltf" ||
        normalized == ".fbx")
      wantsModels = true;
    if (normalized == ".json")
      wantsScenes = true;
  }

  if (wantsImages) {
    for (const std::string &extension : ImageExtensions()) {
      const std::string glob = "*" + extension;
      if (std::find(globs.begin(), globs.end(), glob) == globs.end())
        globs.push_back(glob);
    }
  }

  r = sd_bus_message_open_container(call.message, 'e', "sv");
  if (r < 0)
    return false;
  r = sd_bus_message_append(call.message, "s", "filters");
  if (r < 0)
    return false;
  r = sd_bus_message_open_container(call.message, 'v', "a(sa(us))");
  if (r < 0)
    return false;
  r = sd_bus_message_open_container(call.message, 'a', "(sa(us))");
  if (r < 0)
    return false;

  if (wantsImages) {
    r = AppendNamedFilter(
        call.message, "Images",
        {{0, "*.png"},
         {0, "*.[pP][nN][gG]"},
         {0, "*.jpg"},
         {0, "*.jpeg"},
         {0, "*.webp"},
         {0, "*.bmp"},
         {0, "*.tga"},
         {0, "*.gif"},
         {0, "*.hdr"},
         {1, "image/png"},
         {1, "image/jpeg"},
         {1, "image/webp"},
         {1, "image/bmp"},
         {1, "image/gif"},
         {1, "image/*"}});
    if (r < 0)
      return false;
  } else if (wantsModels) {
    r = AppendNamedFilter(call.message, "3D Models",
                          {{0, "*.obj"},
                           {0, "*.glb"},
                           {0, "*.gltf"},
                           {0, "*.fbx"},
                           {1, "model/gltf-binary"},
                           {1, "model/gltf+json"}});
    if (r < 0)
      return false;
  } else if (wantsScenes) {
    r = AppendNamedFilter(call.message, "JSON Scenes",
                          {{0, "*.json"}, {1, "application/json"}});
    if (r < 0)
      return false;
  } else if (!globs.empty()) {
    std::vector<std::pair<uint32_t, const char *>> rules;
    rules.reserve(globs.size());
    for (const std::string &glob : globs)
      rules.emplace_back(0u, glob.c_str());
    r = AppendNamedFilter(call.message, "Files", rules);
    if (r < 0)
      return false;
  }

  r = AppendNamedFilter(call.message, "All files", {{0, "*"}});
  if (r < 0)
    return false;

  r = sd_bus_message_close_container(call.message);
  if (r < 0)
    return false;
  r = sd_bus_message_close_container(call.message);
  if (r < 0)
    return false;
  r = sd_bus_message_close_container(call.message);
  if (r < 0)
    return false;

  r = sd_bus_message_close_container(call.message);
  if (r < 0)
    return false;

  BusError error;
  BusMessage reply;
  r = sd_bus_call(bus_, call.message, 0, &error.error, &reply.message);
  if (r < 0) {
    std::cerr << "PortalFileChooser: OpenFile failed: "
              << (error.error.message ? error.error.message : strerror(-r))
              << std::endl;
    return false;
  }

  const char *handle = nullptr;
  r = sd_bus_message_read(reply.message, "o", &handle);
  if (r < 0 || !handle)
    return false;

  outHandle = handle;
  return true;
}

bool PortalFileChooser::Open(const std::string &title,
                             const std::vector<std::string> &extensions,
                             const std::filesystem::path &startFolder) {
  if (busy_)
    return false;
  if (!EnsureBus())
    return false;

  completed_ = false;
  cancelled_ = false;
  selected_.clear();

  std::filesystem::path folder = ExistingDirectory(startFolder);
  if (folder.empty())
    folder = ExistingDirectory(lastDirectory_);

  const std::string token = MakeHandleToken(++tokenCounter_);
  const char *uniqueName = nullptr;
  int r = sd_bus_get_unique_name(bus_, &uniqueName);
  if (r < 0 || !uniqueName) {
    std::cerr << "PortalFileChooser: missing unique bus name" << std::endl;
    return false;
  }

  const std::string expectedPath =
      "/org/freedesktop/portal/desktop/request/" +
      SenderToPathElement(uniqueName) + "/" + token;

  if (!SubscribeToResponse(expectedPath))
    return false;

  busy_ = true;
  std::string actualHandle;
  if (!CallOpenFile(title, extensions, folder, token, actualHandle)) {
    ResetRequest();
    return false;
  }

  if (busy_ && !actualHandle.empty() && actualHandle != expectedPath) {
    if (!SubscribeToResponse(actualHandle)) {
      ResetRequest();
      return false;
    }
  }

  Poll();
  return true;
}
