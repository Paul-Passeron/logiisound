#include "imgui_impl_sdl2.h"
#include <imgui.h>
#include <string>

using std::string;

class Editor {

  double zoom = 1.0;
  ImVec2 offset = ImVec2(0, 0);
  ImVec2 windowPos;
  ImVec2 windowSize;
  ImVec2 mousePos;
  bool focused = false;

  double getScaleFactor();
  void renderGrid();

  ImVec2 screenToGrid(const ImVec2 &screenPos, const ImVec2 &windowPos) const;
  ImVec2 gridToScreen(const ImVec2 &gridPos, const ImVec2 &windowPos) const;

  string current_component_id = "";

public:
  void render();
  void handleEvent(SDL_Event event);
  void setComponentId(string id);
};
