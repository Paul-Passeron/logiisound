#include "NPNFactory.hpp"
#include "../../core/Application.hpp"
#include "../models/transistors/BJTs/NPNModel.hpp"
#include <SDL_image.h>
#include <filesystem>
#include <stdexcept>
#include <string>

using std::filesystem::path;

ComponentModel *NPNFactory::fromJson(const json &data, const int *const pins,
                                     const size_t &nPins) {
  if (nPins != 3) {
    throw std::runtime_error("Expected exactly 3 pins in NPN. Got " +
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
  return new NPNModel(pins[0], pins[1], pins[2], value);
}

string NPNFactory::getComponentType() const { return "res"; }
json NPNFactory::getDefaultJson() const {
  // TODO: pool automatically from the NPN model library
  const auto models = NPNModel::getModels();
  return {{"model", {{"value", models[0]}, {"values", models}}}};
}

void *NPNFactory::getTexture() const { return texture; }

pair<int, int> NPNFactory::getSize() const { return {2, 2}; }

const vector<pair<int, int>> &NPNFactory::getPins() const {
  static const vector<pair<int, int>> pins = {{0, -1}, {0, 1}, {-1, 0}};
  return pins;
}

NPNFactory::NPNFactory() {
  if (texture == nullptr) {
    SDL_Renderer *renderer = Application::getInstance()->getRenderer();
    path texturePath =
        std::filesystem::current_path().parent_path() / "assets/icons/npn.png";
    texture = IMG_LoadTexture(renderer, texturePath.c_str());
  }
}

void *NPNFactory::texture = nullptr;
