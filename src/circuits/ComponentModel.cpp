#include "ComponentModel.hpp"
#include <iostream>

void ComponentModel::updateState(const Eigen::VectorXd &V,
                            const Eigen::VectorXd &I) {
  // Do nothing for non transient basic components...
}

void ComponentModel::initializeState() {
  // Do nothing for non transient basic components...
}

void ComponentModel::render(SDL_Renderer *renderer, const SDL_Rect &rect) const {
  if(SDL_RenderCopy(renderer, texture_, nullptr, &rect)){
    std::cerr << "ERROR: " << SDL_GetError() << std::endl;
  }
}

SDL_Texture *ComponentModel::getTexture() const { return texture_; }
