#pragma once

#include "models/ComponentModel.hpp"
#include <filesystem>
#include <functional>
#include <imgui.h>
#include <nlohmann/json.hpp>
#include <string>

using std::vector;
using std::filesystem::path;
using json = nlohmann::json;

struct ComponentInfo {
  std::string name;
  SDL_Texture *previewTexture;
  std::function<ComponentModel *(const json &data, const vector<int> &pins)> createFunction;
  void renderPreview(SDL_Renderer *renderer, const SDL_Rect &rect);
  int xSize = 2.0;
  int ySize = 2.0;
  vector<ImVec2> pins;
  json data;
};

class ComponentRegistry {
public:
  static ComponentRegistry &instance();
  void registerComponent(
      const std::string &type, const std::string &name, const path texturePath,
      std::function<ComponentModel *(const json &data, const vector<int> &pins)>
          createFunction,
      int xSize = 2.0, int ySize = 2.0, vector<ImVec2> pins = vector<ImVec2>(0),
      json data = nullptr);

  const std::unordered_map<std::string, ComponentInfo> &getRegistry() const;
  static ComponentInfo getComponent(std::string id);
  ComponentModel *createComponent(const std::string &type);

private:
  std::unordered_map<std::string, ComponentInfo> registry;
  ComponentRegistry() = default;
};

void registerComponents();
