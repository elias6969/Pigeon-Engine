#pragma once

#include <filesystem>
#include <string>
#include <vector>

class PortalFileChooser {
public:
  static PortalFileChooser &Instance();

  static const std::vector<std::string> &ImageExtensions();
  static const std::vector<std::string> &ModelExtensions();
  static const std::vector<std::string> &SceneExtensions();

  bool Open(const std::string &title, const std::vector<std::string> &extensions,
            const std::filesystem::path &startFolder = {});

  void Poll();
  bool IsBusy() const;
  bool ConsumeCompletion(std::filesystem::path &outPath, bool &cancelled);
  const std::filesystem::path &LastDirectory() const;

  PortalFileChooser(const PortalFileChooser &) = delete;
  PortalFileChooser &operator=(const PortalFileChooser &) = delete;

private:
  PortalFileChooser() = default;
  ~PortalFileChooser();

  bool EnsureBus();
  void ResetRequest();
  bool SubscribeToResponse(const std::string &objectPath);
  bool CallOpenFile(const std::string &title,
                    const std::vector<std::string> &extensions,
                    const std::filesystem::path &folder,
                    const std::string &handleToken,
                    std::string &outHandle);
  static int OnResponse(struct sd_bus_message *message, void *userdata,
                        struct sd_bus_error *error);

  struct sd_bus *bus_ = nullptr;
  struct sd_bus_slot *matchSlot_ = nullptr;
  bool busy_ = false;
  bool completed_ = false;
  bool cancelled_ = false;
  bool unrefMatchAfterCallback_ = false;
  std::filesystem::path selected_;
  std::filesystem::path lastDirectory_;
  unsigned int tokenCounter_ = 0;
};
