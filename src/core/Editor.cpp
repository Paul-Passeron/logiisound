#include <set>
#include <stdexcept>
#include <string>
#define IMGUI_DEFINE_MATH_OPERATORS

#include "../circuits/ComponentRegistry.hpp"
#include "../circuits/factories/NonComponentFactory.hpp"
#include "../circuits/models/VoltageSourceModel.hpp"
#include "CableHelper.hpp"
#include "CircuitSerializer.hpp"
#include "Editor.hpp"
#include <SDL_events.h>
#include <SDL_render.h>
#include <cmath>
#include <imgui.h>
#include <imgui_internal.h>
#include <iostream>

template <typename Base, typename T> inline bool instanceof(const T *ptr) {
  return dynamic_cast<const Base *>(ptr) != nullptr;
}

// TODO:
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
  drawList->AddCircleFilled(gridToScreen({0, 0}), 15, IM_COL32(0, 0, 255, 255));
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
  renderDebugPins();

  if (openCompPopup) {
    ImGui::OpenPopup("comp_rc", ImGuiWindowFlags_NoMove);
    openCompPopup = false;
  }
  if (ImGui::BeginPopup("comp_rc")) {
    renderCompPopup();
    ImGui::EndPopup();
  }

  if (openEditComp) {
    ImGui::OpenPopup("editComp");
    openEditComp = false;
    const PlacedComponent &c = placedComponents[rightClickedComp];
    json data = c.data;
    std::cout << data.dump() << std::endl;
  }

  if (ImGui::BeginPopup("editComp")) {
    PlacedComponent &c = placedComponents[rightClickedComp];
    json &data = c.data;
    ImGui::LabelText("##editCompLabel", "Edit Component");
    ImGui::Separator();
    for (auto &[k, v] : data.items()) {
      string s = k;
      if (s.length() > 0) {
        s[0] = toupper(k[0]);
      }
      ImGui::Text("%s: ", s.c_str());
      ImGui::SameLine();

      if (v["value"].is_number_float() || v["value"].is_number()) {
        float mult = 1.0f;
        // TODO:
        // - Format specified directly in JSON ?
        // - Dynamic prefix
        string fmt = "%.1f ";
        if (s == "R") {
          fmt += " Ohms";
        } else if (s == "C") {
          fmt += " pF";
          mult = 1e12;
        } else if (s == "V") {
          fmt += " V";
        }
        float new_value = v["value"];
        new_value *= mult;
        ImGui::PushID((intptr_t)&v);
        ImGui::SliderFloat("##", &new_value, mult * (float)v["min"],
                           mult * (float)v["max"], fmt.c_str(),
                           ImGuiSliderFlags_Logarithmic);
        ImGui::PopID();
        v["value"] = new_value / mult;
      } else if (v["value"].is_string() && v["values"].is_array() &&
                 !v["values"].empty()) {
        auto &vals = v["values"];
        std::string currentValue = v["value"].get<std::string>();
        int index = 0;
        for (int i = 0; i < vals.size(); i++) {
          if (vals[i].get<std::string>() == currentValue) {
            index = i;
            break;
          }
        }
        std::vector<std::string> items;
        for (auto &v : vals) {
          items.push_back(v.get<std::string>());
        }
        std::vector<const char *> itemsPtr;
        for (const auto &item : items) {
          itemsPtr.push_back(item.c_str());
        }
        ImGui::PushID((intptr_t)&v);
        if (ImGui::Combo("Select Value", &index, itemsPtr.data(),
                         itemsPtr.size())) {
          v["value"] = vals[index];
        }
        ImGui::PopID();
      }
    }
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
    if (event.type == SDL_MOUSEWHEEL) {
      ImVec2 old = screenToGrid(mousePos);
      double oldZoom = zoom;
      if (event.wheel.y > 0)
        zoom *= 1.1;
      else if (event.wheel.y < 0)
        zoom /= 1.1;
      ImVec2 newer = screenToGrid(mousePos);
      offset.x += (newer.x - old.x) * getScaleFactor();
      offset.y += (newer.y - old.y) * getScaleFactor();
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
      openCompPopup = true;
      rightClickedComp = index;
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
  ComponentFactory *comp =
      ComponentRegistry::getComponent(current_component_id);
  pair<int, int> size = comp->getSize();
  int xSize = size.first;
  int ySize = size.second;
  ImageRotated((ImTextureID)comp->getTexture(), snappedScreenPos,
               ImVec2(xSize, ySize) * scaleFactor, angle,
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
  // manager.addCable(lastPoint, actualEnd);
  addCable({lastPoint, actualEnd});
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
    comp.data = ComponentRegistry::getComponent(comp.type)->getDefaultJson();
    addComponent(comp);
  }
}

void Editor::updateCompNodes() {
  std::vector<ImVec2> nodes;
  double scaleFactor = getScaleFactor();
  PointCompare compare;
  for (auto c : placedComponents) {
    ComponentFactory *comp = ComponentRegistry::getComponent(c.type);
    for (const auto &p : comp->getPins()) {
      ImVec2 pin(p.first, p.second);
      ImVec2 actualGridPos = ImRotate(pin, cosf(c.angle * M_PI / 180.0),
                                      sinf(c.angle * M_PI / 180.0));
      actualGridPos += c.position;
      actualGridPos.x = roundf(actualGridPos.x);
      actualGridPos.y = roundf(actualGridPos.y);
      nodes.emplace_back(actualGridPos);
    }
  }
  manager.updateExternalNodes(nodes);
}

void Editor::renderComponents() {
  double scaleFactor = getScaleFactor();
  for (auto c : placedComponents) {
    ComponentFactory *comp = ComponentRegistry::getComponent(c.type);
    pair<int, int> size = comp->getSize();
    ImageRotated((ImTextureID)comp->getTexture(), gridToScreen(c.position),
                 ImVec2(size.first, size.second) * scaleFactor, c.angle);
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
  ComponentFactory *comp = ComponentRegistry::getComponent(c.type);
  float sina = sinf(c.angle * M_PI / 180);
  float cosa = cosf(c.angle * M_PI / 180);
  pair<int, int> size = comp->getSize();
  int xSize = size.first;
  int ySize = size.second;
  ImVec2 va = c.position - ImRotate(ImVec2(xSize, ySize) / 2, cosa, sina);
  ImVec2 vc = c.position + ImRotate(ImVec2(xSize, ySize) / 2, cosa, sina);
  ImVec2 vb = c.position - ImRotate(ImVec2(xSize, -ySize) / 2, cosa, sina);
  ImVec2 vd = c.position + ImRotate(ImVec2(xSize, -ySize) / 2, cosa, sina);
  va = gridToScreen(va);
  vb = gridToScreen(vb);
  vc = gridToScreen(vc);
  vd = gridToScreen(vd);
  ImDrawList *l = ImGui::GetWindowDrawList();
  l->AddLine(va, vb, ImColor(255, 0, 0, 100), 3.0);
  l->AddLine(vb, vc, ImColor(255, 0, 0, 100), 3.0);
  l->AddLine(vc, vd, ImColor(255, 0, 0, 100), 3.0);
  l->AddLine(vd, va, ImColor(255, 0, 0, 100), 3.0);
}

int Editor::getHoveredComponentIndex() {
  ImVec2 mouseGridPos = screenToGrid(mousePos);
  int index = -1;
  for (const auto &c : placedComponents) {
    index++;
    ComponentFactory *comp = ComponentRegistry::getComponent(c.type);
    pair<int, int> size = comp->getSize();
    int xSize = size.first;
    int ySize = size.second;
    ImVec2 b = c.position - ImVec2(xSize, ySize) / 2;
    ImVec2 a = c.position + ImVec2(xSize, ySize) / 2;
    if (mouseGridPos.x <= a.x && mouseGridPos.y <= a.y &&
        mouseGridPos.x >= b.x && mouseGridPos.y >= b.y) {
      return index;
    }
  }
  return -1;
}

void Editor::renderCompPopup() {
  if (placedComponents.size() > rightClickedComp) {

    if (ImGui::Button("Delete")) {
      placedComponents.erase(placedComponents.begin() + rightClickedComp);
      updateCompNodes();
      ImGui::CloseCurrentPopup();
    }
    json data = placedComponents[rightClickedComp].data;
    if (data != nullptr) {
      if (ImGui::Button("Edit")) {
        openEditComp = true;
        ImGui::CloseCurrentPopup();
      }
    }
  }
}

void Editor::renderDebugPins() {
  ImDrawList *l = ImGui::GetWindowDrawList();
  for (const auto &c : placedComponents) {
    const auto &comp = ComponentRegistry::getComponent(c.type);
    for (const auto &p : comp->getPins()) {
      ImVec2 pin(p.first, p.second);
      ImVec2 actualGridPos = ImRotate(pin, cosf(c.angle * M_PI / 180.0),
                                      sinf(c.angle * M_PI / 180.0));
      actualGridPos += c.position;
      actualGridPos.x = roundf(actualGridPos.x);
      actualGridPos.y = roundf(actualGridPos.y);
      l->AddCircleFilled(gridToScreen(actualGridPos), 8,
                         ImColor(0, 255, 0, 50));
    }
  }
}

void Editor::clearCircuit() {
  placedComponents.clear();
  manager.clear();
  updateNodeFamilies();
}

void Editor::addComponent(const PlacedComponent &component) {
  placedComponents.emplace_back(component);
  updateCompNodes();
  updateNodeFamilies();
}

void Editor::addCable(const pair<ImVec2, ImVec2> &cable) {
  manager.addCable(cable.first, cable.second);
  std::cout << "addCable" << std::endl;
  updateNodeFamilies();
}

bool Editor::saveCircuit(const string &filePath) {
  return CircuitSerializer::saveCircuit(*this, filePath);
}

bool Editor::loadCircuit(const string &filePath) {
  bool res = CircuitSerializer::loadCircuit(*this, filePath);
  updateNodeFamilies();
  return res;
}

// TODO: clean this mess up
CircuitProcessor *Editor::toCircuit() {
  std::map<int, std::set<pair<int, int>>> nodeMap;
  std::vector<int> groundFamilies;
  nodeMap[-1] = std::set<pair<int, int>>();
  int nodeCount = 0;
  // First pooling all ground pins
  for (const auto &comp : placedComponents) {
    if (comp.type == "gnd") {
      const auto &c = ComponentRegistry::getComponent(comp.type);
      const auto &p = c->getPins()[0];
      ImVec2 pinV(p.first, p.second);
      ImVec2 actualGridPos = ImRotate(pinV, cosf(comp.angle * M_PI / 180.0),
                                      sinf(comp.angle * M_PI / 180.0));
      actualGridPos += comp.position;
      pair<int, int> pin = {actualGridPos.x, actualGridPos.y};
      int i = 0;
      for (const auto &family : nodeFamilies) {
        bool in = false;
        for (const auto against : family) {
          if (pin.first == against.first && pin.second == against.second) {
            in = true;
            break;
          }
        }
        if (in) {
          groundFamilies.emplace_back(i);
          std::copy(family.begin(), family.end(),
                    std::inserter(nodeMap[-1], nodeMap[-1].end()));
        }
        i++;
      }
    }
  }

  // Then creating all Nodes
  int i = 0;
  for (const auto &family : nodeFamilies) {
    // Make sure it is not connected to ground
    bool in = false;
    for (const auto &val : groundFamilies) {
      if (i == val) {
        in = true;
        break;
      }
    }
    if (!in) {
      nodeMap[nodeCount] = std::set<pair<int, int>>();
      // std::copy(family.begin(), family.end(),
      //           std::inserter(nodeMap[nodeCount], nodeMap[nodeCount].end()));
      for (const auto &v : family) {
        nodeMap[nodeCount].insert(v);
      }
      nodeCount++;
    }
    i++;
  }

  Circuit *circ = new Circuit(nodeCount);
  CircuitProcessor *proc = new CircuitProcessor(circ);

  for (const auto &comp : placedComponents) {
    ComponentFactory *factory = ComponentRegistry::getComponent(comp.type);
    vector<int> pinIndices;
    for (const auto &p : factory->getPins()) {

      ImVec2 pinV(p.first, p.second);
      ImVec2 actualGridPos = ImRotate(pinV, cosf(comp.angle * M_PI / 180.0),
                                      sinf(comp.angle * M_PI / 180.0));
      actualGridPos += comp.position;
      pair<int, int> pin = {roundf(actualGridPos.x), roundf(actualGridPos.y)};
      bool found = false;
      for (const auto &[key, value] : nodeMap) {
        if (value.find(pin) != value.end()) {
          pinIndices.emplace_back(key);
          found = true;
          break;
        }
      }
      if (!found) {
        throw std::runtime_error(
            "Internal Error: Could not find pin index for component " +
            comp.type + " (" + std::to_string(pin.first) + ", " +
            std::to_string(pin.second) + ")");
      }
    }

    bool inputFound = false;
    bool outputFound = false;

    if (instanceof<NonComponentFactory>(factory)) {
      if (comp.type == "gnd") {
        continue;
      } else if (comp.type == "in") {
        if (inputFound) {
          throw std::runtime_error("Mutliple input sources in circuit.");
        }
        inputFound = true;
        VoltageSourceModel *model =
            new VoltageSourceModel(0.0, pinIndices[0], -1);
        int input = circ->addComponent(model);
        proc->setInput(input);
      } else if (comp.type == "out") {
        if (outputFound) {
          throw std::runtime_error("Mutliple outputs in circuit.");
        }
        outputFound = true;
        proc->setOutput(pinIndices[0]);
      }
    }

    ComponentModel *model =
        factory->fromJson(comp.data, pinIndices.data(), pinIndices.size());
    if (model != nullptr) {
      circ->addComponent(model);
    }
  }

  return proc;
}

void Editor::updateNodeFamilies() {
  // Clear existing node families
  for (auto &vecs : nodeFamilies) {
    vecs.clear();
  }
  nodeFamilies.clear();

  // Collect all nodes from cables and component pins
  std::set<pair<int, int>> allNodes;

  // Add nodes from cables
  for (const auto &cable : manager.getCables()) {
    pair<int, int> first = {cable.first.x, cable.first.y};
    pair<int, int> second = {cable.second.x, cable.second.y};
    if ((first.first == 3 && first.second == 4) ||
        (second.first == 3 && second.second == 4)) {
      std::cout << "FOUND 3 4 !!!! " << std::endl;
    }
    allNodes.emplace(first);
    allNodes.emplace(second);
  }

  // Add nodes from component pins
  for (const auto &c : placedComponents) {
    float cos_a = cosf(c.angle * M_PI / 180.0);
    float sin_a = sinf(c.angle * M_PI / 180.0);
    ComponentFactory *f = ComponentRegistry::getComponent(c.type);
    for (const auto &p : f->getPins()) {
      ImVec2 res = ImRotate(ImVec2(p.first, p.second), cos_a, sin_a);
      res += c.position;
      res.x = roundf(res.x);
      res.y = roundf(res.y);
      allNodes.emplace(res.x, res.y);
    }
  }
  // Create adjacency list for the graph of nodes
  std::map<pair<int, int>, std::vector<pair<int, int>>> adjacencyList;

  // Initialize adjacency list
  for (const auto &node : allNodes) {
    adjacencyList.emplace(node, std::vector<pair<int, int>>());
  }

  // Add edges between nodes connected by cables
  for (const auto &cable : manager.getCables()) {
    pair<int, int> first = {cable.first.x, cable.first.y};
    pair<int, int> second = {cable.second.x, cable.second.y};
    adjacencyList[first].emplace_back(second);
    adjacencyList[second].emplace_back(first);
  }

  // Use a set to track visited nodes during DFS
  std::set<pair<int, int>> visited;

  // Helper function for DFS traversal
  std::function<void(const pair<int, int> &, std::vector<pair<int, int>> &)>
      dfs = [&](const pair<int, int> &node,
                std::vector<pair<int, int>> &currentFamily) {
        visited.emplace(node);
        currentFamily.push_back(node);

        for (const auto &neighbor : adjacencyList[node]) {
          if (visited.find(neighbor) == visited.end()) {
            dfs(neighbor, currentFamily);
          }
        }
      };

  // Perform DFS from each unvisited node to find connected components
  for (const auto &node : allNodes) {
    if (visited.find(node) == visited.end()) {
      std::vector<pair<int, int>> currentFamily;
      dfs(node, currentFamily);
      nodeFamilies.emplace_back(currentFamily);
    }
  }
  std::cout << "Total node count: " << nodeFamilies.size() << std::endl;
  if (!verifyNodeFamilies()) {
    std::cout << "FUCK !!!" << std::endl;
  }
}

bool Editor::verifyNodeFamilies() {
  // Collect all nodes from cables and component pins
  std::set<pair<int, int>> allNodes;

  // Add nodes from cables
  for (const auto &cable : manager.getCables()) {
    pair<int, int> first = {cable.first.x, cable.first.y};
    pair<int, int> second = {cable.second.x, cable.second.y};
    allNodes.emplace(first);
    allNodes.emplace(second);
  }

  // Add nodes from component pins
  for (const auto &c : placedComponents) {
    float cos_a = cosf(c.angle * M_PI / 180.0);
    float sin_a = sinf(c.angle * M_PI / 180.0);
    ComponentFactory *f = ComponentRegistry::getComponent(c.type);
    for (const auto &p : f->getPins()) {
      ImVec2 res = ImRotate(ImVec2(p.first, p.second), cos_a, sin_a);
      res += c.position;
      res.x = roundf(res.x);
      res.y = roundf(res.y);
      allNodes.emplace(res.x, res.y);
    }
  }

  // Create a set of all nodes in nodeFamilies
  std::set<pair<int, int>> nodesInFamilies;
  for (const auto &family : nodeFamilies) {
    for (const auto &node : family) {
      nodesInFamilies.emplace(node);
    }
  }

  // Check if sizes match
  if (allNodes.size() != nodesInFamilies.size()) {
    std::cout << "Node count mismatch: " << allNodes.size()
              << " nodes found, but " << nodesInFamilies.size()
              << " nodes in families." << std::endl;

    // Find missing nodes
    std::set<pair<int, int>> missingNodes;
    for (const auto &node : allNodes) {
      if (nodesInFamilies.find(node) == nodesInFamilies.end()) {
        missingNodes.emplace(node);
      }
    }

    // Find extra nodes
    std::set<pair<int, int>> extraNodes;
    for (const auto &node : nodesInFamilies) {
      if (allNodes.find(node) == allNodes.end()) {
        extraNodes.emplace(node);
      }
    }

    // Print detailed information about missing/extra nodes
    if (!missingNodes.empty()) {
      std::cout << "Missing nodes in families:" << std::endl;
      for (const auto &[x, y] : missingNodes) {
        std::cout << "  (" << x << ", " << y << ")" << std::endl;
      }
    }

    if (!extraNodes.empty()) {
      std::cout << "Extra nodes in families:" << std::endl;
      for (const auto &[x, y] : extraNodes) {
        std::cout << "  (" << x << ", " << y << ")" << std::endl;
      }
    }

    return false;
  }

  // Check if all nodes are in families
  for (const auto &node : allNodes) {
    if (nodesInFamilies.find(node) == nodesInFamilies.end()) {
      std::cout << "Node (" << node.first << ", " << node.second
                << ") is missing from nodeFamilies" << std::endl;
      return false;
    }
  }

  // Check if all nodes in families are valid
  for (const auto &node : nodesInFamilies) {
    if (allNodes.find(node) == allNodes.end()) {
      std::cout << "Node (" << node.first << ", " << node.second
                << ") in nodeFamilies doesn't exist in the circuit"
                << std::endl;
      return false;
    }
  }

  // Check if each node appears in exactly one family
  std::map<pair<int, int>, int> nodeFamilyCount;
  for (size_t i = 0; i < nodeFamilies.size(); i++) {
    for (const auto &node : nodeFamilies[i]) {
      nodeFamilyCount[node]++;
      if (nodeFamilyCount[node] > 1) {
        std::cout << "Node (" << node.first << ", " << node.second
                  << ") appears in multiple families" << std::endl;
        return false;
      }
    }
  }

  std::cout << "All " << allNodes.size()
            << " nodes are correctly accounted for in " << nodeFamilies.size()
            << " families." << std::endl;
  return true;
}
