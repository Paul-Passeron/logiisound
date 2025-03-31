#pragma once
#include <SDL_render.h>
#include <eigen3/Eigen/Dense>

#include <vector>

using std::vector;

class ComponentModel {
public:
  virtual void stamp(Eigen::MatrixXd &matrix, Eigen::VectorXd &rhs,
                     double currentTime, double dt) = 0;
  virtual void updateState(const Eigen::VectorXd &V, const Eigen::VectorXd &I);
  virtual void initializeState();
};
