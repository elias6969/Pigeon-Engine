#pragma once

#include <glm/glm.hpp>
#include <nlohmann/json_fwd.hpp>
#include <string>
#include <vector>

class FontManager;

struct DialogueLine {
  std::string text = "Hello Pigeon";
  std::string speaker;
  glm::vec2 position = {80.0f, 40.0f};
  float scale = 1.0f;
  glm::vec3 bodyColor = {0.933f, 0.925f, 0.902f};
  glm::vec3 speakerColor = {0.769f, 0.659f, 0.541f};
  glm::vec3 boxFill = {0.133f, 0.153f, 0.169f};
  glm::vec3 boxBorder = {0.349f, 0.388f, 0.420f};
  float boxAlpha = 0.90f;
  float padding = 28.0f;
  float borderWidth = 1.5f;
  float accentBarWidth = 2.0f;
  float portraitGap = -36.0f;
  bool showContinueIndicator = true;
  glm::vec3 continueColor = {0.647f, 0.678f, 0.698f};
  std::string backgroundPath;
  float backgroundAlpha = 1.0f;
  bool isDialogue = false;
  float boxWidth = 900.0f;
  float boxHeight = 180.0f;
  float charsPerSecond = 28.0f;
  float holdAfterLine = 1.25f;
  int sequence = 0;
  std::string portraitPath;
  float portraitSize = 220.0f;
  bool portraitOnLeft = true;
  glm::vec2 portraitOffset = {0.0f, 0.0f};
  bool speakingBob = true;
  float bobAmount = 8.0f;
  float bobSpeed = 8.0f;
};

class DialogueSystem {
public:
  int AddLine(const DialogueLine &line);
  void UpdateLine(int index, const DialogueLine &line);
  void RemoveLine(int index);
  void Clear();

  void Play();
  void Stop();
  bool IsPlaying() const;
  void SetPreviewLine(int lineIndex);
  void SetSceneBackground(const std::string &path);

  const DialogueLine *GetLine(int index) const;
  std::vector<int> DialogueOrder() const;
  void MoveDialogue(int slot, int direction);

  void Render(FontManager &fonts, int width, int height);

private:
  void RememberSpeaker(const DialogueLine &line);
  std::string ResolvePortraitPath(const DialogueLine &line) const;
  std::string ResolveBackgroundPath(const DialogueLine &line) const;
  unsigned int OverlayTexture(const std::string &path);
  std::vector<int> DialogueIndices() const;
  void UpdatePlayback();
  std::string VisibleText(const DialogueLine &line, bool animate) const;
  bool IsRevealing(const DialogueLine &line) const;
  const DialogueLine *CurrentDialogueLine() const;
  void DrawBackground(FontManager &fonts, const DialogueLine *line, int width,
                      int height);
  void DrawLine(FontManager &fonts, const DialogueLine &line,
                const std::string &visibleText, bool speaking, int width,
                int height);

  std::vector<DialogueLine> lines;
  std::vector<std::pair<std::string, std::string>> speakerPortraits;
  std::vector<std::pair<std::string, unsigned int>> overlayTextures;
  std::string sceneBackgroundPath;

  bool playing = false;
  int playIndex = 0;
  double lineStartTime = 0.0;
  int previewLine = -1;
};

DialogueSystem &GetDialogueSystem();

void WriteDialogueJson(nlohmann::json &out, const DialogueLine &line);
void ReadDialogueJson(const nlohmann::json &in, DialogueLine &line);
