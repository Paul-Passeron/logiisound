#include "ComponentRegistry.hpp"
#include "../core/Application.hpp"
#include "imgui.h"
#include "models/CapacitorModel.hpp"
#include "models/DiodeModel.hpp"
#include "models/ResistorModel.hpp"
#include "models/transistors/BJTs/NPNModel.hpp"
#include <SDL_image.h>
#include <iostream>
#include <stdexcept>

void registerComponents() {
  ComponentRegistry &instance = ComponentRegistry::instance();
  path prefix = std::filesystem::current_path().parent_path() / "assets/icons";
  instance.registerComponent("res", "Resistor", prefix / "resistor.png",
                             []() { return new ResistorModel(1, -1, -1); }, 2,
                             2, {ImVec2(-1, 0), ImVec2(1, 0)},
                             {{"r",
                               {
                                   {"value", 1e3},
                                   {"min", 1},
                                   {"max", 1e8},
                               }}});
  instance.registerComponent("cap", "Capacitor", prefix / "capacitor.png",
                             []() { return new CapacitorModel(1, -1, -1); }, 2,
                             2, {ImVec2(-1, 0), ImVec2(1, 0)},
                             {{"c",
                              {
                                  {"value", 22e-9},
                                  {"min", 1e-12},
                                  {"max", 8e-4},
                              }}});
  instance.registerComponent(
      "diode", "Diode", prefix / "diode.png",
      []() { return new DiodeModel(-1, -1, "1N4148"); }, 2, 2,
      {ImVec2(-1, 0), ImVec2(1, 0)},
      {{"model",
       {{"value", "1N4148"},
        {"values", {"1N4148", "1N34A", "1N5817", "BAT41"}}}}});
  instance.registerComponent(
      "npn", "NPN Transistor", prefix / "npn.png",
      []() { return new NPNModel(-1, -1, -1, "2N3904"); }, 2, 2,
      {ImVec2(-1, 0), ImVec2(0, 1), ImVec2(0, -1)},
      {{"model",
       {{"value", "2N3904"}, {"values", {"2N2222", "BC337", "2N3904"}}}}});

  instance.registerComponent("gnd", "Ground connection", prefix / "ground.png",
                             []() { return nullptr; }, 2, 2, {ImVec2(0, -1)});
  instance.registerComponent("input", "Input connection", prefix / "input.png",
                             []() { return nullptr; }, 2, 2, {ImVec2(1, 0)});
  instance.registerComponent("output", "Output connection",
                             prefix / "output.png", []() { return nullptr; }, 2,
                             2, {ImVec2(-1, 0)});
}

ComponentRegistry &ComponentRegistry::instance() {
  static ComponentRegistry registry;
  return registry;
}

void ComponentRegistry::registerComponent(
    const std::string &type, const std::string &name, const path texturePath,
    std::function<ComponentModel *()> createFunction, int xSize, int ySize,
    vector<ImVec2> pins, json data) {
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

ComponentModel *ComponentRegistry::createComponent(const std::string &type) {
  auto it = registry.find(type);
  return it != registry.end() ? it->second.createFunction() : nullptr;
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
