#include "ComponentRegistry.hpp"
#include "../core/Application.hpp"
#include "imgui.h"
#include <SDL_image.h>
#include <iostream>
#include <stdexcept>

void registerComponents() {
  ComponentRegistry &instance = ComponentRegistry::instance();
  path prefix = std::filesystem::current_path().parent_path() / "assets/icons";
  instance.registerComponent(
      "res", "Resistor", prefix / "resistor.png",
      [](json data, vector<int> pins) { return nullptr; }, 2, 2,
      {ImVec2(-1, 0), ImVec2(1, 0)},
      {{"r",
        {
            {"value", 1e3},
            {"min", 1},
            {"max", 1e8},
        }}});
  instance.registerComponent(
      "cap", "Capacitor", prefix / "capacitor.png",
      [](json data, vector<int> pins) { return nullptr; }, 2, 2,
      {ImVec2(-1, 0), ImVec2(1, 0)},
      {{"c",
        {
            {"value", 22e-9},
            {"min", 1e-12},
            {"max", 8e-4},
        }}});
  instance.registerComponent(
      "diode", "Diode", prefix / "diode.png",
      [](json data, vector<int> pins) { return nullptr; }, 2, 2,
      {ImVec2(-1, 0), ImVec2(1, 0)},
      {{"model",
        {{"value", "1N4148"},
         {"values", {"1N4148", "1N34A", "1N5817", "BAT41"}}}}});
  instance.registerComponent(
      "npn", "NPN Transistor", prefix / "npn.png",
      [](json data, vector<int> pins) { return nullptr; }, 2, 2,
      {ImVec2(-1, 0), ImVec2(0, 1), ImVec2(0, -1)},
      {{"model",
        {{"value", "2N3904"}, {"values", {"2N2222", "BC337", "2N3904"}}}}});

  instance.registerComponent(
      "gnd", "Ground connection", prefix / "ground.png",
      [](json data, vector<int> pins) { return nullptr; }, 2, 2,
      {ImVec2(0, -1)});
  instance.registerComponent(
      "input", "Input connection", prefix / "input.png",
      [](json data, vector<int> pins) { return nullptr; }, 2, 2,
      {ImVec2(1, 0)});
  instance.registerComponent(
      "output", "Output connection", prefix / "output.png",
      [](json data, vector<int> pins) { return nullptr; }, 2, 2,
      {ImVec2(-1, 0)});
}

ComponentRegistry &ComponentRegistry::instance() {
  static ComponentRegistry registry;
  return registry;
}

void ComponentRegistry::registerComponent(
    const std::string &type, const std::string &name, const path texturePath,
    std::function<ComponentModel *(const json &data, const vector<int> &pins)> createFunction,
    int xSize, int ySize, vector<ImVec2> pins, json data) {
  SDL_Renderer *renderer = Application::getInstance()->getRenderer();
  SDL_Texture *tex = IMG_LoadTexture(renderer, texturePath.c_str());
  registry[type] = {name, tex, createFunction, xSize, ySize, pins, data};

  std::cout << "[INFO]: Registered component " << name << " with id " << type
            << std::endl;
}

const std::unordered_map<std::string, ComponentInfo> &
ComponentRegistry::getRegistry() const {
  return registry;
}


void ComponentInfo::renderPreview(SDL_Renderer *renderer,
                                  const SDL_Rect &rect) {
  if (SDL_RenderCopy(renderer, previewTexture, nullptr, &rect)) {
    std::cerr << "ERROR: " << SDL_GetError() << std::endl;
  }
}

ComponentInfo ComponentRegistry::getComponent(std::string id) {
  auto r = instance().getRegistry();
  if (r.find(id) == r.end()) {
    throw std::runtime_error("Unknown component " + id);
  }
  return r.at(id);
}
