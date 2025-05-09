#include "ResistorFactory.hpp"
#include "../../core/Application.hpp"
#include "../models/ResistorModel.hpp"
#include <SDL_image.h>
#include <filesystem>
#include <stdexcept>

using std::filesystem::path;

ComponentModel *ResistorFactory::fromJson(const json &data,
                                          const int *const pins,
                                          const size_t &nPins) {
  if (nPins != 2) {
    throw std::runtime_error("Expected exactly 2 pins in Resistor. Got " +
                             std::to_string(nPins));
  }
  if (!data.contains("r")) {
    throw std::runtime_error("Malformed json: Expected \'r\' field.");
  }
  float r = data["r"]["value"].get<float>();
  return new ResistorModel(r, pins[0], pins[1]);
}

string ResistorFactory::getComponentType() const { return "res"; }
json ResistorFactory::getDefaultJson() const {
  return {{"r",
           {
               {"max", 1e8},
               {"min", 1},
               {"value", 1e3},
           }}};
}

void *ResistorFactory::getTexture() const { return texture; }

pair<int, int> ResistorFactory::getSize() const { return {2, 2}; }

const vector<pair<int, int>> &ResistorFactory::getPins() const {
  static const vector<pair<int, int>> pins = {{-1, 0}, {1, 0}};
  return pins;
}

ResistorFactory::ResistorFactory() {
  if (texture == nullptr) {
    SDL_Renderer *renderer = Application::getInstance()->getRenderer();
    path texturePath = std::filesystem::current_path().parent_path() /
                       "assets/icons/resistor.png";
    texture = IMG_LoadTexture(renderer, texturePath.c_str());
  }
}

void *ResistorFactory::texture = nullptr;
