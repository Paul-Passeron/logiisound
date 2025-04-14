#include "CapacitorFactory.hpp"
#include "../../core/Application.hpp"
#include "../models/CapacitorModel.hpp"
#include <SDL_image.h>
#include <filesystem>
#include <stdexcept>

using std::filesystem::path;

ComponentModel *CapacitorFactory::fromJson(const json &data,
                                           const int *const pins,
                                           const size_t &nPins) {
  if (nPins != 2) {
    throw std::runtime_error("Expected exactly 2 pins in capacitor. Got " +
                             std::to_string(nPins));
  }
  if (!data.contains("c")) {
    throw std::runtime_error("Malformed json: Expected \'c\' field.");
  }
  float c = data["c"].get<float>();
  return new CapacitorModel(c, pins[0], pins[1]);
}

string CapacitorFactory::getComponentType() const { return "cap"; }

json CapacitorFactory::getDefaultJson() const {
  return {{"c",
           {
               {"max", 0.0008},
               {"min", 1e-12},
               {"value", 2.2e-08},
           }}};
}
void *CapacitorFactory::getTexture() const { return texture; }

pair<int, int> CapacitorFactory::getSize() const { return {2, 2}; }

const vector<pair<int, int>> &CapacitorFactory::getPins() const {
  static const vector<pair<int, int>> pins = {{-1, 0}, {1, 0}};
  return pins;
}

CapacitorFactory::CapacitorFactory() {
  if (texture == nullptr) {
    SDL_Renderer *renderer = Application::getInstance()->getRenderer();
    path texturePath = std::filesystem::current_path().parent_path() /
                       "assets/icons/capacitor.png";
    texture = IMG_LoadTexture(renderer, texturePath.c_str());
  }
}

void *CapacitorFactory::texture = nullptr;
