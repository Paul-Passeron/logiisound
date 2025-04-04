#pragma once
#include "imgui_impl_sdl2.h"
#include "CableManager.hpp"
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
  CableManager manager;
  std::vector<PlacedComponent> placedComponents;

  double getScaleFactor() const;
  void renderGrid();

  ImVec2 screenToGrid(const ImVec2 &screenPos) const;
  ImVec2 gridToScreen(const ImVec2 &gridPos) const;

  // Cables
  const static ImU32 wireColor;
  const static ImU32 wirePreviewColor;
  const static ImU32 wireHoverColor;
  const static float hoverDistance;
  void initWire();
  void endWire();
  void renderPreviewWire();
  void renderHoveredWire(int index);
  void renderWires();
  void tryDeleteWire();

  // Components
  string current_component_id;
  int componentCount = 0;
  void renderComponentPreview();
  void placeCurrentComponent();
  void renderComponents();
  void updateCompNodes();
  void renderHoveredComp(int index);
  int getHoveredComponentIndex();
public:
  void render();
  void handleEvent(SDL_Event event);
  void setComponentId(string id);
};
