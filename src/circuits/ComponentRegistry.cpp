#include "ComponentRegistry.hpp"
#include "CapacitorModel.hpp"
#include "DiodeModel.hpp"
#include "ResistorModel.hpp"
#include "transistors/BJTs/NPNModel.hpp"
#include <iostream>

void registerComponents() {
  ComponentRegistry &instance = ComponentRegistry::instance();
  instance.registerComponent("res", "Resistor",
                             []() { return new ResistorModel(1, -1, -1); });
  instance.registerComponent("cap", "Capacitor",
                             []() { return new CapacitorModel(1, -1, -1); });
  instance.registerComponent("diode", "Diode",
                             []() { return new DiodeModel(-1, -1, "1N4148"); });
  instance.registerComponent("npn", "NPN Transistor", []() {
    return new NPNModel(-1, -1, -1, "2N3904");
  });
}

ComponentRegistry &ComponentRegistry::instance() {
  static ComponentRegistry registry;
  return registry;
}

void ComponentRegistry::registerComponent(
    const std::string &type, const std::string &name,
    std::function<ComponentModel *()> createFunction) {
  registry[type] = {name, createFunction};
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
