#include "VoltageSourceFactory.hpp"
#include "../../core/Application.hpp"
#include "../models/VoltageSourceModel.hpp"
#include <SDL_image.h>
#include <filesystem>
#include <stdexcept>

using std::filesystem::path;

ComponentModel *VoltageSourceFactory::fromJson(const json &data,
                                               const int *const pins,
                                               const size_t &nPins) {
  if (nPins != 2) {
    throw std::runtime_error("Expected exactly 2 pins in VoltageSource. Got " +
                             std::to_string(nPins));
  }
  if (!data.contains("v")) {
    throw std::runtime_error("Malformed json: Expected \'v\' field.");
  }
  float v = data["v"].get<float>();
  return new VoltageSourceModel(v, pins[0], pins[1]);
}

string VoltageSourceFactory::getComponentType() const { return "res"; }
json VoltageSourceFactory::getDefaultJson() const {
  return {{"v",
           {
               {"max", 1e4},
               {"min", 1e-4},
               {"value", 5},
           }}};
}

void *VoltageSourceFactory::getTexture() const { return texture; }

pair<int, int> VoltageSourceFactory::getSize() const { return {2, 2}; }

vector<pair<int, int>> VoltageSourceFactory::getPins() const {
  return {{-1, 0}, {1, 0}};
}

VoltageSourceFactory::VoltageSourceFactory() {
  if (texture == nullptr) {
    SDL_Renderer *renderer = Application::getInstance()->getRenderer();
    path texturePath = std::filesystem::current_path().parent_path() /
                       "assets/icons/voltagesource.png";
    texture = IMG_LoadTexture(renderer, texturePath.c_str());
  }
}

void *VoltageSourceFactory::texture = nullptr;
