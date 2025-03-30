#include "Editor.hpp"
#include "imgui.h"
#include <SDL_events.h>
#include <cmath>
#include <iostream>

double Editor::getScaleFactor() { return 10.0 * zoom; }

void Editor::renderGrid() {
  double scaleFactor = getScaleFactor();
  ImColor gridColor = IM_COL32(100, 100, 100, 150);
  ImDrawList *drawList = ImGui::GetWindowDrawList();
  for (float x = fmod(offset.x, scaleFactor); x < windowSize.x;
       x += scaleFactor) {
    drawList->AddLine(ImVec2(windowPos.x + x, windowPos.y),
                      ImVec2(windowPos.x + x, windowPos.y + windowSize.y),
                      gridColor, 1.2f);
  }

  for (float y = fmod(offset.y, scaleFactor); y < windowSize.y;
       y += scaleFactor) {
    drawList->AddLine(ImVec2(windowPos.x, windowPos.y + y),
                      ImVec2(windowPos.x + windowSize.x, windowPos.y + y),
                      gridColor, 1.2f);
  }
}

void Editor::render() {
  ImGui::Begin("Editor", nullptr);
  windowPos = ImGui::GetWindowPos();
  windowSize = ImGui::GetWindowSize();
  mousePos = ImGui::GetMousePos();
  focused = ImGui::IsWindowFocused();
  renderGrid();

  double scaleFactor = getScaleFactor();

  ImVec2 mouseGridPos = screenToGrid(mousePos, windowPos);
  ImVec2 snappedGridPos =
      ImVec2(roundf(mouseGridPos.x), roundf(mouseGridPos.y));
  ImVec2 snappedScreenPos = gridToScreen(snappedGridPos, windowPos);
  ImDrawList *drawList = ImGui::GetWindowDrawList();
  if (focused) {
    drawList->AddCircleFilled(snappedScreenPos, 3.0f, IM_COL32(255, 0, 0, 255));
  }

  ImGui::End();
}

void Editor::handleEvent(SDL_Event event) {
  if (!focused) {
    return;
  }
  if (event.type == SDL_MOUSEWHEEL) {
    if (event.wheel.y > 0)
      zoom *= 1.1;
    else if (event.wheel.y < 0)
      zoom /= 1.1;
  }
  if (event.type == SDL_MOUSEMOTION &&
      (event.motion.state & SDL_BUTTON_MMASK)) {
    offset.x += event.motion.xrel;
    offset.y += event.motion.yrel;
  }
}

ImVec2 Editor::screenToGrid(const ImVec2 &screenPos,
                            const ImVec2 &windowPos) const {
  float scaleFactor = 10.0f * zoom;
  return ImVec2((screenPos.x - windowPos.x - offset.x) / scaleFactor,
                (screenPos.y - windowPos.y - offset.y) / scaleFactor);
}

ImVec2 Editor::gridToScreen(const ImVec2 &gridPos,
                            const ImVec2 &windowPos) const {
  float scaleFactor = 10.0f * zoom;
  return ImVec2(gridPos.x * scaleFactor + offset.x + windowPos.x,
                gridPos.y * scaleFactor + offset.y + windowPos.y);
}

void Editor::setComponentId(string id) { current_component_id = id; std::cout << "Current component is: " << id << std::endl;}
