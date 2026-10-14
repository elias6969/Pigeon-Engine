# Dialogue Story System

Notes for a later story graph. Sequential playback in `DialogueSystem` stays as it is. Do not add choice UI, flags, or branching until this is implemented on purpose.

## Data

Keep a line as data, not as an editor object index.

```cpp
enum class StoryNodeKind { Line, Choice };

struct StoryLine {
  std::string id;          // "momo_intro_01"
  std::string speaker;     // "Momo"
  std::string text;
  std::string portrait;    // optional override; otherwise CharacterRecord
  std::string next;        // id of the next line, empty = end
};

struct StoryChoice {
  std::string text;
  std::string next;        // jump target
  std::string setFlag;     // optional, empty = no write
  bool flagValue = true;
};

struct StoryNode {
  StoryNodeKind kind = StoryNodeKind::Line;
  StoryLine line;
  std::vector<StoryChoice> choices; // only used when kind == Choice
};

struct StoryScene {
  std::string id;          // "market_intro"
  std::string start;       // first node id
  std::vector<StoryNode> nodes;
};

struct CharacterRecord {
  std::string name;        // "Momo"
  std::string portrait;    // default picture for every line by this speaker
};
```

`DialogueLine` in the editor can stay the visual-novel box (position, colors, portrait bob). A story scene only supplies speaker, text, portrait, and what comes next. Playback copies the current story line into the active `DialogueLine` instead of walking every editor object.

Portraits belong on `CharacterRecord`, keyed by speaker name. A line portrait is only an override (angry Momo, off-screen Momo). The current speaker map in `DialogueSystem::RememberSpeaker` is the small version of that table.

## Runtime

One state machine. `Play(sceneId)` is the only entry.

```cpp
struct StoryState {
  const StoryScene* scene = nullptr;
  std::string nodeId;
  std::unordered_map<std::string, bool> flags;
  bool waitingForChoice = false;
};

void Play(const std::string& sceneId);
void Advance();                 // timer, click, or key
void Choose(int choiceIndex);   // later
```

Per frame:

1. If the active node is a `Line`, reveal text the way `DialogueSystem` already does.
2. When the typewriter finishes and hold time elapses, `Advance()` follows `line.next`.
3. If `next` is empty, stop.
4. If the next node is a `Choice`, stop auto-advance and set `waitingForChoice`. That branch is not built yet.

Current `DialogueSystem::Play()` can stay the editor preview: it walks `isDialogue` lines in order, which is the same as every line's `next` pointing at the following line.

## Choices later

A choice node does not need a second renderer. After the previous line is fully shown:

- Draw the same dialogue box.
- Draw one button per `StoryChoice`.
- `Choose(i)` writes `flags[setFlag] = flagValue` when `setFlag` is not empty, then sets `nodeId = choice.next` and clears `waitingForChoice`.

Conditions, when they are needed, are a filter in front of `next`, not a new playback loop:

```cpp
struct StoryEdge {
  std::string next;
  std::string requireFlag; // empty = always
  bool requireValue = true;
};

std::string ResolveNext(const StoryLine& line, const StoryState& state) {
  for (const StoryEdge& edge : line.edges) {
    if (edge.requireFlag.empty())
      return edge.next;
    auto it = state.flags.find(edge.requireFlag);
    if (it != state.flags.end() && it->second == edge.requireValue)
      return edge.next;
  }
  return {};
}
```

Until edges exist, `StoryLine::next` is enough. Adding `edges` later does not change `UpdatePlayback` beyond swapping the "index + 1" step for `ResolveNext`.

## Save

Store scenes as JSON next to the editor scene, not inside every cube record:

```json
{
  "id": "market_intro",
  "start": "momo_01",
  "characters": [{ "name": "Momo", "portrait": "assets/portraits/momo.png" }],
  "nodes": [
    { "kind": "line", "id": "momo_01", "speaker": "Momo", "text": "...", "next": "choice_01" },
    {
      "kind": "choice",
      "id": "choice_01",
      "choices": [
        { "text": "Follow him", "next": "follow_01", "setFlag": "followed_momo" },
        { "text": "Stay", "next": "stay_01", "setFlag": "followed_momo", "flagValue": false }
      ]
    }
  ]
}
```

Editor objects keep owning box layout. Story files own order, speakers, and future jumps.
