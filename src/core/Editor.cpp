#define IMGUI_DEFINE_MATH_OPERATORS
#include "Editor.hpp"
#include "../circuits/ComponentRegistry.hpp"
#include "CableHelper.hpp"
#include <SDL_events.h>
#include <SDL_render.h>
#include <cmath>
#include <imgui.h>
#include <imgui_internal.h>
#include <iostream>
// TODO:
// - maybe outsource cable management to a distinct class ?
// - Be able to interact with placed components

const float Editor::hoverDistance = 0.3f;

const char *getStateString(EditorState s) {
  switch (s) {
  case WireState:
    return "WireState";
  case WireDrawing:
    return "WireDrawing";
  case ComponentState:
    return "ComponentState";
  case RotateState:
    return "RotateState";
  default:
    return "Unknown State (Error)";
  }
}

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
  ImGui::Text("Current State: %s", getStateString(state));
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

  int compIndex = getHoveredComponentIndex();
  renderHoveredComp(compIndex);

  if (state == ComponentState || state == RotateState) {
    renderComponentPreview();
  } else if (state == WireDrawing) {
    renderPreviewWire();
  }

  if (compIndex < 0) {
    float distance;
    int cableIndex = manager.findNearestCable(mouseGridPos, &distance);
    if (cableIndex >= 0 && distance <= hoverDistance) {
      renderHoveredWire(cableIndex);
    }
  }

  renderComponents();
  if (openCompPopup) {
    ImGui::OpenPopup("comp_rc", ImGuiWindowFlags_NoMove);
    openCompPopup = false;
  }
  if (ImGui::BeginPopup("comp_rc")) {
    renderCompPopup();
    ImGui::EndPopup();
  }

  ImGui::End();
}

void Editor::tryDeleteWire() {
  ImVec2 mouseGridPos = screenToGrid(mousePos);
  float distance;
  int cableIndex = manager.findNearestCable(mouseGridPos, &distance);
  if (cableIndex >= 0 && distance <= hoverDistance) {
    manager.deleteCable(cableIndex);
  }
}

void Editor::handleEvent(SDL_Event event) {
  if (focused) {
    if (event.type == SDL_MOUSEWHEEL) {
      // TODO: figure out how to zoom to cursor, for the moment it
      // is stuck to the top left corner of the grid.
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
    int index = getHoveredComponentIndex();
    if (index < 0) {

      if (state == WireState) {
        tryDeleteWire();
      } else {
        previousState = state;
        state = WireState;
      }
    } else {
      // open a popup ?
      openCompPopup = true;
      rightClickedComp = index;
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
        if (state == ComponentState) {
          previousState = state;
          state = RotateState;
        }
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

void ImageRotated(ImTextureID tex_id, ImVec2 center, ImVec2 size, float angle,
                  ImU32 color = IM_COL32_WHITE) {
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
                          uvs[1], uvs[2], uvs[3], color);
}

void Editor::renderComponentPreview() {
  double scaleFactor = getScaleFactor();
  ImVec2 mouseGridPos = screenToGrid(mousePos);
  ImVec2 snappedGridPos =
      ImVec2(roundf(mouseGridPos.x), roundf(mouseGridPos.y));
  ImVec2 snappedScreenPos = gridToScreen(snappedGridPos);
  ComponentInfo comp = ComponentRegistry::getComponent(current_component_id);
  ImageRotated((ImTextureID)comp.previewTexture, snappedScreenPos,
               ImVec2(comp.xSize, comp.ySize) * scaleFactor, angle,
               ImColor(255, 255, 255, 150));
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
  manager.addCable(lastPoint, actualEnd);
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
  draw_list->AddLine(rA, rB, wirePreviewColor, 2.0f);
}

void Editor::renderWires() {
  ImDrawList *draw_list = ImGui::GetWindowDrawList();
  for (auto [a, b] : manager.getCables()) {
    ImVec2 rA = gridToScreen(a);
    ImVec2 rB = gridToScreen(b);
    draw_list->AddLine(rA, rB, wireColor, 2.0f);
  }
  for (auto a : manager.getJunctionNodes()) {
    ImVec2 rA = gridToScreen(a);
    draw_list->AddCircleFilled(rA, 4.0f, wireColor);
  }
}

const ImU32 Editor::wireColor = IM_COL32(0, 0, 0, 255);
const ImU32 Editor::wirePreviewColor = IM_COL32(25, 200, 25, 255);
const ImU32 Editor::wireHoverColor = IM_COL32(120, 100, 175, 255);

void Editor::placeCurrentComponent() {
  // TODO: check for conflicts
  int compIndex = getHoveredComponentIndex();
  if (compIndex < 0) {
    PlacedComponent comp;
    comp.angle = angle;
    comp.id = componentCount++;
    ImVec2 mouseGridPos = screenToGrid(mousePos);
    comp.position = ImVec2(roundf(mouseGridPos.x), roundf(mouseGridPos.y));
    comp.type = current_component_id;
    placedComponents.emplace_back(comp);
    updateCompNodes();
  }
}

void Editor::updateCompNodes() {
  std::vector<ImVec2> nodes;
  double scaleFactor = getScaleFactor();
  PointCompare compare;
  for (auto c : placedComponents) {
    ComponentInfo comp = ComponentRegistry::getComponent(c.type);
    for (const auto &p : comp.pins) {
      std::cout << c.angle << std::endl;
      ImVec2 actualGridPos = ImRotate(p, cosf(c.angle * M_PI / 180.0),
                                      sinf(c.angle * M_PI / 180.0));
      actualGridPos += c.position;
      actualGridPos.x = roundf(actualGridPos.x);
      actualGridPos.y = roundf(actualGridPos.y);
      bool isIn = false;
      for (const auto &n : nodes) {
        if (compare(n, actualGridPos) < 10e-2) {
          isIn = true;
          break;
        }
      }
      if (!isIn) {
        nodes.emplace_back(actualGridPos);
      }
    }
  }
  manager.updateExternalNodes(nodes);
}

void Editor::renderComponents() {
  double scaleFactor = getScaleFactor();
  for (auto c : placedComponents) {
    ComponentInfo comp = ComponentRegistry::getComponent(c.type);
    ImageRotated((ImTextureID)comp.previewTexture, gridToScreen(c.position),
                 ImVec2(comp.xSize, comp.ySize) * scaleFactor, c.angle);
  }
}

void Editor::renderHoveredWire(int index) {
  if (index < 0) {
    return;
  }
  auto [a, b] = manager.getCables()[index];
  ImVec2 rA = gridToScreen(a);
  ImVec2 rB = gridToScreen(b);
  ImDrawList *draw_list = ImGui::GetWindowDrawList();
  draw_list->AddLine(rA, rB, wireHoverColor, 2.0f);
}

void Editor::renderHoveredComp(int index) {
  if (index < 0) {
    return;
  }
  PlacedComponent c = placedComponents[index];
  ComponentInfo comp = ComponentRegistry::getComponent(c.type);
  float sina = sinf(c.angle * M_PI / 180);
  float cosa = cosf(c.angle * M_PI / 180);
  ImVec2 va =
      ImRotate(c.position - ImVec2(comp.xSize, comp.ySize) / 2, cosa, sina);
  ImVec2 vc =
      ImRotate(c.position + ImVec2(comp.xSize, comp.ySize) / 2, cosa, sina);
  ImVec2 vb =
      ImRotate(c.position - ImVec2(comp.xSize, -comp.ySize) / 2, cosa, sina);
  ImVec2 vd =
      ImRotate(c.position + ImVec2(comp.xSize, -comp.ySize) / 2, cosa, sina);
  va = gridToScreen(va);
  vb = gridToScreen(vb);
  vc = gridToScreen(vc);
  vd = gridToScreen(vd);
  ImDrawList *l = ImGui::GetWindowDrawList();
  l->AddLine(va, vb, ImColor(255, 0, 0), 3.0);
  l->AddLine(vb, vc, ImColor(255, 0, 0), 3.0);
  l->AddLine(vc, vd, ImColor(255, 0, 0), 3.0);
  l->AddLine(vd, va, ImColor(255, 0, 0), 3.0);
}

int Editor::getHoveredComponentIndex() {
  ImVec2 mouseGridPos = screenToGrid(mousePos);
  int index = -1;
  for (const auto &c : placedComponents) {
    index++;
    const ComponentInfo &comp = ComponentRegistry::getComponent(c.type);
    ImVec2 b = c.position - ImVec2(comp.xSize, comp.ySize) / 2;
    ImVec2 a = c.position + ImVec2(comp.xSize, comp.ySize) / 2;
    if (mouseGridPos.x <= a.x && mouseGridPos.y <= a.y &&
        mouseGridPos.x >= b.x && mouseGridPos.y >= b.y) {
      return index;
    }
  }
  return -1;
}

void Editor::renderCompPopup() {
  if(ImGui::Button("Delete")){
    placedComponents.erase(placedComponents.begin() + rightClickedComp);
    updateCompNodes();
    ImGui::CloseCurrentPopup();
  }
}
