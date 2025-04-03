#define IMGUI_DEFINE_MATH_OPERATORS
#include "Editor.hpp"
#include "../circuits/ComponentRegistry.hpp"
#include "imgui.h"
#include "imgui_internal.h"
#include <SDL_events.h>
#include <SDL_render.h>
#include <cmath>
#include <iostream>

// TODO:
// - maybe outsource cable management to a distinct class ?
// - Be able to interact with placed components

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
  ImGui::Text("Current State: %s", state == WireState     ? "WireState"
                                   : state == WireDrawing ? "WireDrawing"
                                                          : "ComponentState");
  windowPos = ImGui::GetWindowPos();
  windowSize = ImGui::GetWindowSize();
  mousePos = ImGui::GetMousePos();
  focused = ImGui::IsWindowHovered();
  renderGrid();

  double scaleFactor = getScaleFactor();

  ImVec2 mouseGridPos = screenToGrid(mousePos);
  ImVec2 snappedGridPos =
      ImVec2(roundf(mouseGridPos.x), roundf(mouseGridPos.y));
  ImVec2 snappedScreenPos = gridToScreen(snappedGridPos);
  ImDrawList *drawList = ImGui::GetWindowDrawList();
  if (focused && (state == WireDrawing || state == WireState)) {
    drawList->AddCircleFilled(snappedScreenPos, 3.0f, IM_COL32(255, 0, 0, 255));
  }
  renderWires();
  renderComponents();

  if (state == ComponentState || state == RotateState) {
    renderComponentPreview();
  } else if (state == WireDrawing) {
    renderPreviewWire();
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
    if (event.type == SDL_MOUSEBUTTONDOWN &&
        (event.button.button == SDL_BUTTON_LEFT)) {
      if (state == WireState) {
        initWire();
      } else if (state == WireDrawing) {
        endWire();
      } else if (state == ComponentState) {
        placeCurrentComponent();
      } else if (state == RotateState) {
        angle += 90.0;
        if (angle > 360.0) {
          angle -= 360.0;
        }
      }
    }
  }
  if (event.type == SDL_MOUSEBUTTONDOWN &&
      (event.button.button == SDL_BUTTON_RIGHT)) {
    if (state == WireState) {
      ImVec2 mouseGridPos = screenToGrid(mousePos);
      float distance;
      int cableIndex = findNearestCable(mouseGridPos, &distance);
      if (cableIndex >= 0 && distance <= 0.1f) {
        deleteCable(cableIndex);
      }
    } else {
      previousState = state;
      state = WireState;
    }
  }
  if (event.type == SDL_KEYDOWN) {
    if (event.key.keysym.sym == SDLK_w) {
      previousState = state;
      state = WireState;
    } else if (event.key.keysym.sym == SDLK_r) {
      if (state == RotateState) {
        state = previousState;
      } else {
        previousState = state;
        state = RotateState;
      }
    }
  }
}

ImVec2 Editor::screenToGrid(const ImVec2 &screenPos) const {
  float scaleFactor = getScaleFactor();
  return ImVec2((screenPos.x - windowPos.x - offset.x) / scaleFactor,
                (screenPos.y - windowPos.y - offset.y) / scaleFactor);
}

ImVec2 Editor::gridToScreen(const ImVec2 &gridPos) const {
  float scaleFactor = getScaleFactor();
  return ImVec2(gridPos.x * scaleFactor + offset.x + windowPos.x,
                gridPos.y * scaleFactor + offset.y + windowPos.y);
}

void Editor::setComponentId(string id) {
  angle = 0;
  previousState = state;
  state = ComponentState;
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
  ImVec2 mouseGridPos = screenToGrid(mousePos);
  ImVec2 snappedGridPos =
      ImVec2(roundf(mouseGridPos.x), roundf(mouseGridPos.y));
  ImVec2 snappedScreenPos = gridToScreen(snappedGridPos);
  ComponentInfo comp = ComponentRegistry::getComponent(current_component_id);
  ImageRotated((ImTextureID)comp.previewTexture, snappedScreenPos,
               ImVec2(comp.xSize, comp.ySize) * scaleFactor, angle);
}

void Editor::initWire() {
  if (state == WireState) {
    previousState = state;
    state = WireDrawing;
    ImVec2 mouseGridPos = screenToGrid(mousePos);
    lastPoint = ImVec2(roundf(mouseGridPos.x), roundf(mouseGridPos.y));
  }
}

ImVec2 getNextPoint(const ImVec2 &first, const ImVec2 &second) {
  int xA = first.x;
  int xB = second.x;
  int yA = first.y;
  int yB = second.y;
  if (xA == xB && yA == yB) {
    return first;
  }
  if (abs(xA - xB) > abs(yA - yB)) {
    yB = yA;
  } else {
    xB = xA;
  }
  return ImVec2(xB, yB);
}

void Editor::endWire() {
  ImVec2 mouseGridPos = screenToGrid(mousePos);
  ImVec2 end = ImVec2(roundf(mouseGridPos.x), roundf(mouseGridPos.y));
  ImVec2 actualEnd = getNextPoint(lastPoint, end);
  if (actualEnd == lastPoint) {
    return;
  }
  cables.emplace_back(lastPoint, actualEnd);
  lastPoint = actualEnd;
}

void Editor::renderPreviewWire() {
  ImVec2 mouseGridPos = screenToGrid(mousePos);
  ImVec2 end = ImVec2(roundf(mouseGridPos.x), roundf(mouseGridPos.y));
  ImVec2 actualEnd = getNextPoint(lastPoint, end);
  if (actualEnd == lastPoint) {
    return;
  }
  ImDrawList *draw_list = ImGui::GetWindowDrawList();
  ImVec2 rA = gridToScreen(lastPoint);
  ImVec2 rB = gridToScreen(actualEnd);
  draw_list->AddLine(rA, rB, previewColor, 2.0f);
}

void Editor::renderWires() {
  ImDrawList *draw_list = ImGui::GetWindowDrawList();
  for (auto [a, b] : cables) {
    ImVec2 rA = gridToScreen(a);
    ImVec2 rB = gridToScreen(b);
    draw_list->AddLine(rA, rB, cableColor, 2.0f);
  }
}

ImU32 Editor::cableColor = IM_COL32(0, 0, 0, 255);
ImU32 Editor::previewColor = IM_COL32(25, 200, 25, 255);

void Editor::placeCurrentComponent() {
  // TODO: check for conflicts
  PlacedComponent comp;
  comp.angle = angle;
  comp.id = componentCount++;
  ImVec2 mouseGridPos = screenToGrid(mousePos);
  comp.position = ImVec2(roundf(mouseGridPos.x), roundf(mouseGridPos.y));
  comp.type = current_component_id;
  placedComponents.emplace_back(comp);
}

void Editor::renderComponents() {
  double scaleFactor = getScaleFactor();
  for (auto c : placedComponents) {
    ComponentInfo comp = ComponentRegistry::getComponent(c.type);
    ImageRotated((ImTextureID)comp.previewTexture, gridToScreen(c.position),
                 ImVec2(comp.xSize, comp.ySize) * scaleFactor, c.angle);
  }
}

float Editor::calculateDistanceToSegment(const ImVec2 &p, const ImVec2 &a,
                                         const ImVec2 &b) const {
  ImVec2 ab = ImVec2(b.x - a.x, b.y - a.y);
  ImVec2 ap = ImVec2(p.x - a.x, p.y - a.y);

  float ab_squared = ab.x * ab.x + ab.y * ab.y;
  float ap_dot_ab = ap.x * ab.x + ap.y * ab.y;

  float t = std::max(0.0f, std::min(1.0f, ap_dot_ab / ab_squared));

  ImVec2 closest = ImVec2(a.x + t * ab.x, a.y + t * ab.y);
  float dx = p.x - closest.x;
  float dy = p.y - closest.y;

  return std::sqrt(dx * dx + dy * dy);
}

int Editor::findNearestCable(const ImVec2 &point, float *outDistance) const {
  int nearestIndex = -1;
  float minDistance = FLT_MAX;

  for (int i = 0; i < cables.size(); i++) {
    const auto &[a, b] = cables[i];
    float distance = calculateDistanceToSegment(point, a, b);

    if (distance < minDistance) {
      minDistance = distance;
      nearestIndex = i;
    }
  }

  if (outDistance)
    *outDistance = minDistance;

  return nearestIndex;
}

void Editor::deleteCable(int index) {
  if (index >= 0 && index < cables.size()) {
    cables.erase(cables.begin() + index);
  }
}
