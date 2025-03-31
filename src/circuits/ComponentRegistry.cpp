#include "ComponentRegistry.hpp"
#include "../core/Application.hpp"
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
                             []() { return new ResistorModel(1, -1, -1); });
  instance.registerComponent("cap", "Capacitor", prefix / "capacitor.png",
                             []() { return new CapacitorModel(1, -1, -1); });
  instance.registerComponent("diode", "Diode", prefix / "diode.png",
                             []() { return new DiodeModel(-1, -1, "1N4148"); });
  instance.registerComponent("npn", "NPN Transistor", prefix / "npn.png", []() {
    return new NPNModel(-1, -1, -1, "2N3904");
  });
}

ComponentRegistry &ComponentRegistry::instance() {
  static ComponentRegistry registry;
  return registry;
}

void ComponentRegistry::registerComponent(
    const std::string &type, const std::string &name, const path texturePath,
    std::function<ComponentModel *()> createFunction, int xSize, int ySize) {
  SDL_Renderer *renderer = Application::getInstance()->getRenderer();
  SDL_Texture *tex = IMG_LoadTexture(renderer, texturePath.c_str());
  registry[type] = {name, tex, createFunction, xSize, ySize};
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
