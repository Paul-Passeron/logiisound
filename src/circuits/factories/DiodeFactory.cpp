#include "DiodeFactory.hpp"
#include "../../core/Application.hpp"
#include "../models/DiodeModel.hpp"
#include <SDL_image.h>
#include <filesystem>
#include <stdexcept>
#include <string>

using std::filesystem::path;

ComponentModel *DiodeFactory::fromJson(const json &data, const int *const pins,
                                       const size_t &nPins) {
  if (nPins != 2) {
    throw std::runtime_error("Expected exactly 2 pins in Diode. Got " +
                             std::to_string(nPins));
  }
  if (!data.contains("model")) {
    throw std::runtime_error("Malformed json: Expected \'model\' field.");
  }
  json model = data["model"];
  if (!model.contains("value")) {
    throw std::runtime_error(
        "Malformed json: Expected \'value\' field in \'model\'.");
  }
  string value = data["value"];
  return new DiodeModel(pins[0], pins[1], value);
}

string DiodeFactory::getComponentType() const { return "res"; }
json DiodeFactory::getDefaultJson() const {
  // TODO: pool automatically from the diode model library
  const auto models = DiodeModel::getModels();
  return {{"model", {{"value", models[0]}, {"values", models}}}};
}

void *DiodeFactory::getTexture() const { return texture; }

pair<int, int> DiodeFactory::getSize() const { return {2, 2}; }

vector<pair<int, int>> DiodeFactory::getPins() const {
  return {{-1, 0}, {1, 0}};
}

void *DiodeFactory::texture = nullptr;

DiodeFactory::DiodeFactory() {
  if (texture == nullptr) {
    SDL_Renderer *renderer = Application::getInstance()->getRenderer();
    path texturePath = std::filesystem::current_path().parent_path() /
                       "assets/icons/resistor.png";
    texture = IMG_LoadTexture(renderer, texturePath.c_str());
  }
}
