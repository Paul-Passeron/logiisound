#pragma once

#include "Component.hpp"

class Resistor : public Component {
  double resistance;
  int node1, node2;

public:
  Resistor(double r, int n1, int n2);
  void stamp(Eigen::MatrixXd &matrix, Eigen::VectorXd &rhs, double currentTime, double dt) override;
};
