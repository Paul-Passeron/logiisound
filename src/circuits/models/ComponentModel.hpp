#pragma once
#include "nlohmann/json_fwd.hpp"
#include <SDL_render.h>
#include <eigen3/Eigen/Dense>
#include <nlohmann/json.hpp>
#include <vector>

using json = nlohmann::json;

using std::vector;

class ComponentModel {
public:
  virtual void fromJson(json data) = 0;
  virtual void stamp(Eigen::MatrixXd &matrix, Eigen::VectorXd &rhs,
                     double currentTime, double dt) = 0;
  virtual void updateState(const Eigen::VectorXd &V, const Eigen::VectorXd &I);
  virtual void initializeState();
};
