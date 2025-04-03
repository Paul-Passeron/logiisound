#include "imgui_impl_sdl2.h"
#include <imgui.h>
#include <string>
#include <vector>

using std::pair;
using std::string;
using std::vector;

typedef enum {
  WireState,
  WireDrawing,
  ComponentState,
  RotateState,
  // ...
} EditorState;

struct PlacedComponent {
  string type;
  ImVec2 position;
  float angle;
  int id;
};

class Editor {

  double zoom = 1.0;
  ImVec2 offset = ImVec2(0, 0);
  ImVec2 windowPos;
  ImVec2 windowSize;
  ImVec2 mousePos;
  bool focused = false;

  EditorState state = WireState;
  EditorState previousState = WireState;
  double angle = 0.0;
  ImVec2 lastPoint;
  vector<pair<ImVec2, ImVec2>> cables;
  std::vector<PlacedComponent> placedComponents;

  double getScaleFactor() const;
  void renderGrid();

  ImVec2 screenToGrid(const ImVec2 &screenPos) const;
  ImVec2 gridToScreen(const ImVec2 &gridPos) const;

  // Cables
  static ImU32 cableColor;
  static ImU32 previewColor;
  void initWire();
  void endWire();
  void renderPreviewWire();
  void renderWires();
  float calculateDistanceToSegment(const ImVec2& p, const ImVec2& a, const ImVec2& b) const;
  int findNearestCable(const ImVec2& point, float* outDistance = nullptr) const;
  void deleteCable(int index);

  // Components
  string current_component_id;
  int componentCount = 0;
  void renderComponentPreview();
  void placeCurrentComponent();
  void renderComponents();

public:
  void render();
  void handleEvent(SDL_Event event);
  void setComponentId(string id);
};
