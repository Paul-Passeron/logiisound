#pragma once
#include <SDL_render.h>
#include <eigen3/Eigen/Dense>

#include <vector>

using std::vector;

class ComponentModel {
protected:
  SDL_Texture *texture_;

public:
  virtual void stamp(Eigen::MatrixXd &matrix, Eigen::VectorXd &rhs,
                     double currentTime, double dt) = 0;
  virtual void updateState(const Eigen::VectorXd &V, const Eigen::VectorXd &I);
  virtual void initializeState();
  SDL_Texture *getTexture() const;
  void render(SDL_Renderer *renderer, const SDL_Rect &rect) const;
};
