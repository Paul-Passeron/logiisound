#include "ComponentRegistry.hpp"
#include "../core/Application.hpp"
#include "factories/CapacitorFactory.hpp"
#include "factories/ComponentFactory.hpp"
#include "factories/DiodeFactory.hpp"
#include "factories/NPNFactory.hpp"
#include "factories/NonComponentFactory.hpp"
#include "factories/ResistorFactory.hpp"
#include "factories/VoltageSourceFactory.hpp"
#include "imgui.h"
#include "models/CapacitorModel.hpp"
#include <SDL_image.h>
#include <filesystem>
#include <iostream>
#include <stdexcept>

ComponentRegistry &ComponentRegistry::instance() {
  static ComponentRegistry registry;
  return registry;
}

void ComponentRegistry::registerFactory(const string &id,
                                        ComponentFactory *factory) {
  std::cout << "Registered component " << id << std::endl;
  registry.emplace(id, factory);
}

const std::unordered_map<std::string, ComponentFactory *> &
ComponentRegistry::getRegistry() const {
  return registry;
}

ComponentFactory *ComponentRegistry::getComponent(std::string id) {
  auto r = instance().getRegistry();
  if (r.find(id) == r.end()) {
    throw std::runtime_error("Unknown component " + id);
  }
  return r.at(id);
}

void registerComponents() {
  ComponentRegistry &reg = ComponentRegistry::instance();
  reg.registerFactory("npn", new NPNFactory());
  reg.registerFactory("diode", new DiodeFactory());
  reg.registerFactory("res", new ResistorFactory());
  reg.registerFactory("cap", new CapacitorFactory());
  reg.registerFactory("src", new VoltageSourceFactory());
  path prefix = std::filesystem::current_path().parent_path() / "assets/icons";
  std::cout << prefix << std::endl;
  reg.registerFactory(
      "gnd", new NonComponentFactory(prefix / "ground.png", {{0, -1}}));
  reg.registerFactory("in",
                      new NonComponentFactory(prefix / "input.png", {{1, 0}}));
  reg.registerFactory("out",
                      new NonComponentFactory(prefix / "output.png", {{-1, 0}}));
}
