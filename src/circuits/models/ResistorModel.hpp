#pragma once

#include "ComponentModel.hpp"

class ResistorModel : public ComponentModel {
  double resistance;
  int node1, node2;

public:
  ResistorModel(double r, int n1, int n2);
  void stamp(Eigen::MatrixXd &matrix, Eigen::VectorXd &rhs, double currentTime, double dt) override;
};
