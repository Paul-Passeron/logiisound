#pragma once

#include "factories/ComponentFactory.hpp"
#include "models/ComponentModel.hpp"
#include <filesystem>
#include <functional>
#include <imgui.h>
#include <nlohmann/json.hpp>
#include <string>

using std::vector;
using std::filesystem::path;
using json = nlohmann::json;

class ComponentRegistry {
public:
  static ComponentRegistry &instance();
  void registerFactory(const string &id, ComponentFactory *factory);

  const std::unordered_map<std::string, ComponentFactory *> &
  getRegistry() const;
  static ComponentFactory *getComponent(std::string id);
  ComponentModel *createComponent(const std::string &type);

protected:
  std::unordered_map<std::string, ComponentFactory *> registry;
  ComponentRegistry() = default;
};

void registerComponents();
