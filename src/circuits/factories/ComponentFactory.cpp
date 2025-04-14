#include "ComponentFactory.hpp"
#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_render.h>

void ComponentFactory::renderPreview(SDL_Renderer *renderer,
                                  const SDL_Rect &rect) {
  if (SDL_RenderCopy(renderer, getTexture(), nullptr, &rect)) {
    std::cerr << "ERROR: " << SDL_GetError() << std::endl;
  }
}
