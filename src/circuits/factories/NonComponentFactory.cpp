#include "NonComponentFactory.hpp"
#include "../../core/Application.hpp"
#include <SDL_image.h>

std::unordered_map<path, void *> NonComponentFactory::textureMap;

NonComponentFactory::NonComponentFactory(path texturePath,
                                         vector<pair<int, int>> pins)
    : texturePath(texturePath), pins(pins) {
  if (textureMap.find(texturePath) == textureMap.end()) {
    SDL_Renderer *renderer = Application::getInstance()->getRenderer();
    void *texture = IMG_LoadTexture(renderer, texturePath.c_str());
    textureMap.emplace(texturePath, texture);
  }
}

ComponentModel *NonComponentFactory::fromJson(const json &data, const int *pins,
                         const size_t &nPins) {
  return nullptr;
}

string NonComponentFactory::getComponentType() const { return "noncomponent"; }

json NonComponentFactory::getDefaultJson() const { return nullptr; }

void *NonComponentFactory::getTexture() const {
  return textureMap[texturePath];
}

pair<int, int> NonComponentFactory::getSize() const { return {2, 2}; }

const vector<pair<int, int>> &NonComponentFactory::getPins() const {
  return pins;
}

NonComponentFactory::~NonComponentFactory() {
  pins.clear();
  textureMap.clear();
  texturePath = "";
}
