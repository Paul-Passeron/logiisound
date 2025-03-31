#define IMGUI_DEFINE_MATH_OPERATORS
#include "Editor.hpp"
#include "../circuits/ComponentRegistry.hpp"
#include "imgui.h"
#include "imgui_internal.h"
#include <SDL_events.h>
#include <SDL_render.h>
#include <cmath>
#include <iostream>

double Editor::getScaleFactor() const { return 50.0 * zoom; }

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

  if (current_component_id != "") {
    renderComponentPreview();
  }

  ImGui::End();
}

void Editor::handleEvent(SDL_Event event) {
  if (focused) {
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
  if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_a) {
    angle += 90.0;
    if (angle > 360.0) {
      angle -= 360.0;
    }
  }
}

ImVec2 Editor::screenToGrid(const ImVec2 &screenPos,
                            const ImVec2 &windowPos) const {
  float scaleFactor = getScaleFactor();
  return ImVec2((screenPos.x - windowPos.x - offset.x) / scaleFactor,
                (screenPos.y - windowPos.y - offset.y) / scaleFactor);
}

ImVec2 Editor::gridToScreen(const ImVec2 &gridPos,
                            const ImVec2 &windowPos) const {
  float scaleFactor = getScaleFactor();
  return ImVec2(gridPos.x * scaleFactor + offset.x + windowPos.x,
                gridPos.y * scaleFactor + offset.y + windowPos.y);
}

void Editor::setComponentId(string id) {
  current_component_id = id;
  std::cout << "Current component is: " << id << std::endl;
}

ImVec2 add(const ImVec2 &a, const ImVec2 &b) {
  return ImVec2(a.x + b.x, a.y + b.y);
}

void ImageRotated(ImTextureID tex_id, ImVec2 center, ImVec2 size, float angle) {
  ImDrawList *draw_list = ImGui::GetWindowDrawList();

  float cos_a = cosf(angle * M_PI / 180.0);
  float sin_a = sinf(angle * M_PI / 180.0);
  ImVec2 pos[4] = {
      center + ImRotate(ImVec2(-size.x * 0.5f, -size.y * 0.5f), cos_a, sin_a),
      center + ImRotate(ImVec2(+size.x * 0.5f, -size.y * 0.5f), cos_a, sin_a),
      center + ImRotate(ImVec2(+size.x * 0.5f, +size.y * 0.5f), cos_a, sin_a),
      center + ImRotate(ImVec2(-size.x * 0.5f, +size.y * 0.5f), cos_a, sin_a)};
  ImVec2 uvs[4] = {ImVec2(0.0f, 0.0f), ImVec2(1.0f, 0.0f), ImVec2(1.0f, 1.0f),
                   ImVec2(0.0f, 1.0f)};

  draw_list->AddImageQuad(tex_id, pos[0], pos[1], pos[2], pos[3], uvs[0],
                          uvs[1], uvs[2], uvs[3], IM_COL32_WHITE);
}

void Editor::renderComponentPreview() {
  double scaleFactor = getScaleFactor();
  ImVec2 mouseGridPos = screenToGrid(mousePos, windowPos);
  ImVec2 snappedGridPos =
      ImVec2(roundf(mouseGridPos.x), roundf(mouseGridPos.y));
  ImVec2 snappedScreenPos = gridToScreen(snappedGridPos, windowPos);
  ComponentInfo comp = ComponentRegistry::getComponent(current_component_id);
  ImageRotated((ImTextureID)comp.previewTexture, snappedScreenPos,
               ImVec2(comp.xSize, comp.ySize) * scaleFactor, angle);
}
