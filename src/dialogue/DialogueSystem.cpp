#include "DialogueSystem.h"

#include "fontManager.h"
#include "textureManager.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cmath>
#include <nlohmann/json.hpp>

namespace {

glm::vec3 ReadColor(const nlohmann::json &value, const glm::vec3 &fallback) {
  if (!value.is_array() || value.size() < 3)
    return fallback;
  return glm::vec3(value[0].get<float>(), value[1].get<float>(),
                   value[2].get<float>());
}

glm::vec2 ReadVec2(const nlohmann::json &value, const glm::vec2 &fallback) {
  if (!value.is_array() || value.size() < 2)
    return fallback;
  return glm::vec2(value[0].get<float>(), value[1].get<float>());
}

} // namespace

DialogueSystem &GetDialogueSystem() {
  static DialogueSystem system;
  return system;
}

void WriteDialogueJson(nlohmann::json &out, const DialogueLine &line) {
  out["text"] = line.text;
  out["speaker"] = line.speaker;
  out["textScale"] = line.scale;
  out["bodyColor"] = {line.bodyColor.r, line.bodyColor.g, line.bodyColor.b};
  out["speakerColor"] = {line.speakerColor.r, line.speakerColor.g,
                         line.speakerColor.b};
  out["boxFill"] = {line.boxFill.r, line.boxFill.g, line.boxFill.b};
  out["boxBorder"] = {line.boxBorder.r, line.boxBorder.g, line.boxBorder.b};
  out["boxAlpha"] = line.boxAlpha;
  out["padding"] = line.padding;
  out["borderWidth"] = line.borderWidth;
  out["accentBarWidth"] = line.accentBarWidth;
  out["portraitGap"] = line.portraitGap;
  out["showContinueIndicator"] = line.showContinueIndicator;
  out["continueColor"] = {line.continueColor.r, line.continueColor.g,
                          line.continueColor.b};
  out["backgroundPath"] = line.backgroundPath;
  out["backgroundAlpha"] = line.backgroundAlpha;
  out["charsPerSecond"] = line.charsPerSecond;
  out["holdAfterLine"] = line.holdAfterLine;
  out["sequence"] = line.sequence;
  out["boxWidth"] = line.boxWidth;
  out["boxHeight"] = line.boxHeight;
  out["portraitPath"] = line.portraitPath;
  out["portraitSize"] = line.portraitSize;
  out["portraitOnLeft"] = line.portraitOnLeft;
  out["portraitOffset"] = {line.portraitOffset.x, line.portraitOffset.y};
  out["speakingBob"] = line.speakingBob;
  out["bobAmount"] = line.bobAmount;
  out["bobSpeed"] = line.bobSpeed;
}

void ReadDialogueJson(const nlohmann::json &in, DialogueLine &line) {
  line.text = in.value("text", line.text);
  line.speaker = in.value("speaker", line.speaker);
  line.scale = in.value("textScale", line.scale);
  if (in.contains("bodyColor"))
    line.bodyColor = ReadColor(in["bodyColor"], line.bodyColor);
  if (in.contains("speakerColor"))
    line.speakerColor = ReadColor(in["speakerColor"], line.speakerColor);
  if (in.contains("boxFill"))
    line.boxFill = ReadColor(in["boxFill"], line.boxFill);
  if (in.contains("boxBorder"))
    line.boxBorder = ReadColor(in["boxBorder"], line.boxBorder);
  line.boxAlpha = in.value("boxAlpha", line.boxAlpha);
  line.padding = in.value("padding", line.padding);
  line.borderWidth = in.contains("borderWidth") ? in["borderWidth"].get<float>()
                                                : 6.0f;
  line.accentBarWidth =
      in.contains("accentBarWidth") ? in["accentBarWidth"].get<float>() : 0.0f;
  line.portraitGap =
      in.contains("portraitGap") ? in["portraitGap"].get<float>() : 16.0f;
  line.showContinueIndicator =
      in.value("showContinueIndicator", line.showContinueIndicator);
  if (in.contains("continueColor"))
    line.continueColor = ReadColor(in["continueColor"], line.continueColor);
  line.backgroundPath = in.value("backgroundPath", line.backgroundPath);
  line.backgroundAlpha = in.value("backgroundAlpha", line.backgroundAlpha);
  line.charsPerSecond = in.value("charsPerSecond", line.charsPerSecond);
  line.holdAfterLine = in.value("holdAfterLine", line.holdAfterLine);
  line.sequence = in.value("sequence", line.sequence);
  line.boxWidth = in.value("boxWidth", line.boxWidth);
  line.boxHeight = in.value("boxHeight", line.boxHeight);
  line.portraitPath = in.value("portraitPath", line.portraitPath);
  line.portraitSize = in.value("portraitSize", line.portraitSize);
  line.portraitOnLeft = in.value("portraitOnLeft", line.portraitOnLeft);
  if (in.contains("portraitOffset"))
    line.portraitOffset = ReadVec2(in["portraitOffset"], line.portraitOffset);
  line.speakingBob = in.value("speakingBob", line.speakingBob);
  line.bobAmount = in.value("bobAmount", line.bobAmount);
  line.bobSpeed = in.value("bobSpeed", line.bobSpeed);
}

int DialogueSystem::AddLine(const DialogueLine &line) {
  DialogueLine stored = line;
  if (stored.isDialogue) {
    int maxSequence = -1;
    bool sequenceTaken = false;
    for (const DialogueLine &existing : lines) {
      if (!existing.isDialogue)
        continue;
      maxSequence = std::max(maxSequence, existing.sequence);
      if (existing.sequence == stored.sequence)
        sequenceTaken = true;
    }
    if (sequenceTaken)
      stored.sequence = maxSequence + 1;
  }

  RememberSpeaker(stored);
  lines.push_back(stored);
  return static_cast<int>(lines.size()) - 1;
}

void DialogueSystem::UpdateLine(int index, const DialogueLine &line) {
  if (index < 0 || index >= static_cast<int>(lines.size()))
    return;
  lines[index] = line;
  RememberSpeaker(line);
}

void DialogueSystem::RemoveLine(int index) {
  if (index < 0 || index >= static_cast<int>(lines.size()))
    return;
  lines.erase(lines.begin() + index);
  if (previewLine == index)
    previewLine = -1;
  else if (previewLine > index)
    previewLine--;
}

void DialogueSystem::Clear() {
  lines.clear();
  speakerPortraits.clear();
  sceneBackgroundPath.clear();
  previewLine = -1;
  Stop();
}

void DialogueSystem::Play() {
  playing = true;
  playIndex = 0;
  lineStartTime = glfwGetTime();
}

void DialogueSystem::Stop() {
  playing = false;
  playIndex = 0;
  lineStartTime = 0.0;
}

bool DialogueSystem::IsPlaying() const { return playing; }

void DialogueSystem::SetPreviewLine(int lineIndex) { previewLine = lineIndex; }

void DialogueSystem::SetSceneBackground(const std::string &path) {
  sceneBackgroundPath = path;
}

void DialogueSystem::RememberSpeaker(const DialogueLine &line) {
  if (line.speaker.empty() || line.portraitPath.empty())
    return;

  for (auto &entry : speakerPortraits) {
    if (entry.first == line.speaker) {
      entry.second = line.portraitPath;
      return;
    }
  }
  speakerPortraits.emplace_back(line.speaker, line.portraitPath);
}

std::string DialogueSystem::ResolvePortraitPath(const DialogueLine &line) const {
  if (!line.portraitPath.empty())
    return line.portraitPath;

  for (const auto &entry : speakerPortraits) {
    if (entry.first == line.speaker)
      return entry.second;
  }
  return {};
}

std::string
DialogueSystem::ResolveBackgroundPath(const DialogueLine &line) const {
  std::string found = sceneBackgroundPath;
  const std::vector<int> indices = DialogueIndices();
  for (int idx : indices) {
    if (!lines[idx].backgroundPath.empty())
      found = lines[idx].backgroundPath;
    if (&lines[idx] == &line)
      break;
  }
  if (!line.backgroundPath.empty())
    found = line.backgroundPath;
  return found;
}

unsigned int DialogueSystem::OverlayTexture(const std::string &path) {
  if (path.empty())
    return 0;

  for (const auto &entry : overlayTextures) {
    if (entry.first == path)
      return entry.second;
  }

  const unsigned int texture = loadTexture(path.c_str());
  if (texture == 0)
    return 0;

  overlayTextures.emplace_back(path, texture);
  return texture;
}

const DialogueLine *DialogueSystem::GetLine(int index) const {
  if (index < 0 || index >= static_cast<int>(lines.size()))
    return nullptr;
  return &lines[index];
}

std::vector<int> DialogueSystem::DialogueOrder() const {
  return DialogueIndices();
}

void DialogueSystem::MoveDialogue(int slot, int direction) {
  const std::vector<int> order = DialogueIndices();
  const int target = slot + direction;
  if (slot < 0 || target < 0 || slot >= static_cast<int>(order.size()) ||
      target >= static_cast<int>(order.size()))
    return;

  std::swap(lines[order[slot]].sequence, lines[order[target]].sequence);
}

std::vector<int> DialogueSystem::DialogueIndices() const {
  std::vector<int> indices;
  for (int i = 0; i < static_cast<int>(lines.size()); ++i) {
    if (lines[i].isDialogue)
      indices.push_back(i);
  }

  std::stable_sort(indices.begin(), indices.end(),
                   [&](int a, int b) {
                     return lines[a].sequence < lines[b].sequence;
                   });
  return indices;
}

void DialogueSystem::UpdatePlayback() {
  const std::vector<int> indices = DialogueIndices();
  if (!playing || indices.empty())
    return;

  if (playIndex < 0 || playIndex >= static_cast<int>(indices.size()))
    playIndex = 0;

  const DialogueLine &line = lines[indices[playIndex]];
  const float cps = std::max(1.0f, line.charsPerSecond);
  const float elapsed = static_cast<float>(glfwGetTime() - lineStartTime);
  const float typeTime =
      line.text.empty() ? 0.0f : static_cast<float>(line.text.size()) / cps;
  const float lineDuration = typeTime + std::max(0.0f, line.holdAfterLine);

  if (elapsed >= lineDuration &&
      playIndex + 1 < static_cast<int>(indices.size())) {
    playIndex++;
    lineStartTime = glfwGetTime();
  }
}

std::string DialogueSystem::VisibleText(const DialogueLine &line,
                                        bool animate) const {
  if (!animate || line.text.empty())
    return line.text;

  const float elapsed = static_cast<float>(glfwGetTime() - lineStartTime);
  const float cps = std::max(1.0f, line.charsPerSecond);
  const auto revealed = static_cast<size_t>(std::max(0.0f, elapsed * cps));
  return line.text.substr(0, std::min(revealed, line.text.size()));
}

bool DialogueSystem::IsRevealing(const DialogueLine &line) const {
  if (line.text.empty())
    return false;

  const float elapsed = static_cast<float>(glfwGetTime() - lineStartTime);
  const float cps = std::max(1.0f, line.charsPerSecond);
  return elapsed * cps < static_cast<float>(line.text.size());
}

const DialogueLine *DialogueSystem::CurrentDialogueLine() const {
  const std::vector<int> indices = DialogueIndices();
  if (indices.empty())
    return nullptr;

  if (playing) {
    int index = playIndex;
    if (index < 0 || index >= static_cast<int>(indices.size()))
      index = 0;
    return &lines[indices[index]];
  }

  int shown = previewLine;
  if (shown < 0 || shown >= static_cast<int>(lines.size()) ||
      !lines[shown].isDialogue)
    shown = indices.front();
  return &lines[shown];
}

void DialogueSystem::DrawBackground(FontManager &fonts, const DialogueLine *line,
                                    int width, int height) {
  std::string path = sceneBackgroundPath;
  float alpha = 1.0f;
  if (line) {
    path = ResolveBackgroundPath(*line);
    alpha = std::clamp(line->backgroundAlpha, 0.0f, 1.0f);
  }

  const unsigned int texture = OverlayTexture(path);
  if (texture == 0 || width <= 0 || height <= 0)
    return;

  fonts.RenderTexture(texture, 0.0f, 0.0f, static_cast<float>(width),
                      static_cast<float>(height), glm::vec3(1.0f), alpha);
}

void DialogueSystem::DrawLine(FontManager &fonts, const DialogueLine &line,
                              const std::string &visibleText, bool speaking,
                              int width, int height) {
  if (!line.isDialogue) {
    fonts.SetFont(FontFace::Display);
    fonts.RenderText(visibleText, line.position.x, line.position.y, line.scale,
                     line.bodyColor);
    return;
  }

  fonts.SetFont(FontFace::Body);

  const float margin = 48.0f;
  const float screenW = static_cast<float>(std::max(1, width));
  const float screenH = static_cast<float>(std::max(1, height));
  float boxW = std::max(200.0f, line.boxWidth);
  float boxH = std::max(80.0f, line.boxHeight);
  float panelX = line.position.x;
  float panelY = line.position.y;

  if (panelX < margin)
    panelX = margin;
  if (panelX + boxW > screenW - margin)
    boxW = std::max(200.0f, screenW - margin - panelX);
  if (panelY < margin)
    panelY = margin;
  if (panelY + boxH > screenH - margin)
    panelY = std::max(margin, screenH - margin - boxH);

  const float padding = std::max(0.0f, line.padding);
  const float border = std::max(0.0f, line.borderWidth);
  const float accent = std::max(0.0f, line.accentBarWidth);
  const float fillAlpha = std::clamp(line.boxAlpha, 0.0f, 1.0f);

  fonts.RenderRect(panelX, panelY - 6.0f, boxW, boxH, glm::vec3(0.0f), 0.28f);
  if (border > 0.0f)
    fonts.RenderRect(panelX - border, panelY - border, boxW + border * 2.0f,
                     boxH + border * 2.0f, line.boxBorder,
                     std::clamp(fillAlpha + 0.08f, 0.0f, 1.0f));
  fonts.RenderRect(panelX, panelY, boxW, boxH, line.boxFill, fillAlpha);
  if (accent > 0.0f)
    fonts.RenderRect(panelX, panelY, accent, boxH, line.speakerColor, 1.0f);

  const std::string portraitPath = ResolvePortraitPath(line);
  const unsigned int portrait = OverlayTexture(portraitPath);
  const float portraitSize = std::max(32.0f, line.portraitSize);
  float overlapInsetL = 0.0f;
  float overlapInsetR = 0.0f;
  if (portrait != 0 && line.portraitGap < 0.0f) {
    const float overlap = std::min(portraitSize, -line.portraitGap);
    if (line.portraitOnLeft)
      overlapInsetL = overlap;
    else
      overlapInsetR = overlap;
  }

  const float textX = panelX + padding + accent + overlapInsetL;
  const float wrapWidth =
      std::max(40.0f, boxW - padding * 2.0f - accent - overlapInsetL -
                          overlapInsetR);
  float cursorY = panelY + boxH - padding - fonts.LineHeight(line.scale) * 0.35f;

  if (!line.speaker.empty()) {
    const float speakerScale = line.scale * 0.70f;
    const float tracking = 1.8f;
    fonts.RenderText(line.speaker, textX, cursorY, speakerScale,
                     line.speakerColor, tracking);
    const float nameWidth =
        fonts.MeasureTextWidth(line.speaker, speakerScale, tracking);
    fonts.RenderRect(textX, cursorY - 4.0f, nameWidth, 1.0f, line.speakerColor,
                     0.85f);
    cursorY -= fonts.LineHeight(line.scale);
  }

  fonts.RenderTextWrapped(visibleText, textX, cursorY, line.scale,
                          line.bodyColor, wrapWidth);

  if (line.showContinueIndicator && !speaking) {
    const float pulse =
        0.35f +
        0.65f * (0.5f + 0.5f * std::sin(static_cast<float>(glfwGetTime()) * 3.5f));
    fonts.RenderRect(panelX + boxW - padding - 12.0f, panelY + padding, 12.0f,
                     2.0f, line.continueColor, pulse);
  }

  if (portrait != 0) {
    float bob = 0.0f;
    if (speaking && line.speakingBob)
      bob = std::sin(static_cast<float>(glfwGetTime()) * line.bobSpeed) *
            line.bobAmount;

    float portraitX = line.portraitOnLeft
                          ? panelX - portraitSize - line.portraitGap
                          : panelX + boxW + line.portraitGap;
    portraitX += line.portraitOffset.x;
    const float portraitY = panelY + line.portraitOffset.y + bob;
    fonts.RenderTexture(portrait, portraitX, portraitY, portraitSize,
                        portraitSize, glm::vec3(1.0f), 1.0f);
  }
}

void DialogueSystem::Render(FontManager &fonts, int width, int height) {
  fonts.SetScreenSize(static_cast<float>(width), static_cast<float>(height));
  UpdatePlayback();

  const DialogueLine *current = CurrentDialogueLine();
  DrawBackground(fonts, current, width, height);

  for (const DialogueLine &line : lines) {
    if (line.isDialogue)
      continue;
    DrawLine(fonts, line, line.text, false, width, height);
  }

  if (!current)
    return;

  if (playing)
    DrawLine(fonts, *current, VisibleText(*current, true), IsRevealing(*current),
             width, height);
  else
    DrawLine(fonts, *current, current->text, false, width, height);
}
