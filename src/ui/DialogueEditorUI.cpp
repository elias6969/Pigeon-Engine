#include "DialogueEditorUI.h"

#include <imgui.h>

#include <cstdio>
#include <vector>

namespace {

bool EditString(const char *label, std::string &value, size_t bufSize,
                bool multiline) {
  std::vector<char> buffer(bufSize, '\0');
  std::snprintf(buffer.data(), buffer.size(), "%s", value.c_str());

  const bool changed =
      multiline ? ImGui::InputTextMultiline(label, buffer.data(), buffer.size(),
                                            ImVec2(-1.0f, 80.0f))
                : ImGui::InputText(label, buffer.data(), buffer.size());
  if (changed)
    value = buffer.data();
  return changed;
}

void EditColor(const char *label, glm::vec3 &color) {
  float values[3] = {color.r, color.g, color.b};
  if (ImGui::ColorEdit3(label, values))
    color = glm::vec3(values[0], values[1], values[2]);
}

void DrawPortraitFields(DialogueLine &line,
                        const std::function<void()> &browsePortrait,
                        const char *browseId) {
  EditString("Portrait Path", line.portraitPath, 512, false);
  ImGui::SameLine();
  if (ImGui::Button(browseId))
    browsePortrait();
  ImGui::DragFloat("Portrait Size", &line.portraitSize, 1.0f, 32.0f, 800.0f,
                   "%.0f");
  ImGui::Checkbox("Portrait On Left", &line.portraitOnLeft);
  ImGui::DragFloat("Portrait Gap", &line.portraitGap, 1.0f, -200.0f, 200.0f,
                   "%.0f");
  ImGui::TextDisabled("Negative gap overlaps the panel.");
  ImGui::DragFloat2("Portrait Offset", &line.portraitOffset.x, 1.0f, -800.0f,
                    800.0f, "%.0f");
  ImGui::Checkbox("Speaking Bob", &line.speakingBob);
  ImGui::DragFloat("Bob Amount", &line.bobAmount, 0.1f, 0.0f, 40.0f, "%.1f");
  ImGui::DragFloat("Bob Speed", &line.bobSpeed, 0.1f, 0.0f, 30.0f, "%.1f");
}

void DrawBackgroundFields(DialogueLine &line,
                          const std::function<void()> &browseBackground,
                          const char *browseId) {
  EditString("Background Path", line.backgroundPath, 512, false);
  ImGui::SameLine();
  if (ImGui::Button(browseId))
    browseBackground();
  ImGui::SliderFloat("Background Alpha", &line.backgroundAlpha, 0.0f, 1.0f,
                     "%.2f");
  ImGui::TextDisabled("Empty path keeps the previous or scene background.");
}

void DrawDialogueFields(DialogueLine &line, bool isDialogue,
                        const std::function<void()> &browsePortrait,
                        const std::function<void()> &browseBackground,
                        const char *browsePortraitId,
                        const char *browseBackgroundId) {
  EditString(isDialogue ? "Dialogue Text" : "Text", line.text, 2048, true);
  ImGui::SliderFloat("Text Scale", &line.scale, 0.1f, 5.0f);
  if (!isDialogue)
    return;

  EditString("Speaker", line.speaker, 128, false);
  EditColor("Speaker Color", line.speakerColor);
  EditColor("Box Fill", line.boxFill);
  EditColor("Box Border", line.boxBorder);
  ImGui::SliderFloat("Box Alpha", &line.boxAlpha, 0.0f, 1.0f, "%.2f");
  ImGui::DragFloat("Padding", &line.padding, 1.0f, 0.0f, 120.0f, "%.0f");
  ImGui::DragFloat("Border Width", &line.borderWidth, 0.1f, 0.0f, 12.0f,
                   "%.1f");
  ImGui::DragFloat("Accent Bar Width", &line.accentBarWidth, 0.1f, 0.0f, 16.0f,
                   "%.1f");
  ImGui::Checkbox("Continue Indicator", &line.showContinueIndicator);
  EditColor("Continue Color", line.continueColor);
  ImGui::SliderFloat("Typewriter Speed", &line.charsPerSecond, 8.0f, 80.0f,
                     "%.0f chars/sec");
  ImGui::DragFloat("Hold After Line", &line.holdAfterLine, 0.05f, 0.0f, 8.0f,
                   "%.2f s");
  ImGui::DragFloat("Box Width", &line.boxWidth, 1.0f, 200.0f, 2000.0f, "%.0f");
  ImGui::DragFloat("Box Height", &line.boxHeight, 1.0f, 80.0f, 600.0f, "%.0f");
  ImGui::Separator();
  ImGui::Text("Scene Background");
  DrawBackgroundFields(line, browseBackground, browseBackgroundId);
  ImGui::Separator();
  ImGui::Text("Character Portrait");
  DrawPortraitFields(line, browsePortrait, browsePortraitId);
}

} // namespace

DialogueLine &DialogueCreationDraft() {
  static DialogueLine draft;
  return draft;
}

void DrawDialogueCreationFields(bool isDialogue,
                                const std::function<void()> &browsePortrait,
                                const std::function<void()> &browseBackground) {
  ImGui::PushID("DialogueCreate");
  DrawDialogueFields(DialogueCreationDraft(), isDialogue, browsePortrait,
                     browseBackground, "Browse##CreatePortrait",
                     "Browse##CreateBackground");
  ImGui::PopID();
}

void DrawDialogueEditFields(DialogueLine &line, bool isDialogue,
                            const std::function<void()> &browsePortrait,
                            const std::function<void()> &browseBackground) {
  ImGui::PushID("DialogueEdit");
  DrawDialogueFields(line, isDialogue, browsePortrait, browseBackground,
                     "Browse##EditPortrait", "Browse##EditBackground");
  ImGui::PopID();
}
