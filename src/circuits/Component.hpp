#pragma once
#include <eigen3/Eigen/Dense>

#include <vector>

using std::vector;

class Component {
public:
  virtual void stamp(Eigen::MatrixXd &matrix, Eigen::VectorXd &rhs,
                     double currentTime, double dt) = 0;
  virtual void updateState(const Eigen::VectorXd &V, const Eigen::VectorXd &I);
  virtual void initializeState();
};
