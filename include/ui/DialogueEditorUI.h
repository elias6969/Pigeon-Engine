#pragma once

#include "DialogueSystem.h"

#include <functional>

DialogueLine &DialogueCreationDraft();

void DrawDialogueCreationFields(bool isDialogue,
                                const std::function<void()> &browsePortrait,
                                const std::function<void()> &browseBackground);

void DrawDialogueEditFields(DialogueLine &line, bool isDialogue,
                            const std::function<void()> &browsePortrait,
                            const std::function<void()> &browseBackground);
