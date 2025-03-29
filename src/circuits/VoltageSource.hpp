#pragma once

#include "Component.hpp"

class VoltageSource : public Component {
protected:
  double voltage;
  int posNode, negNode;
  bool called = false;
  int index = -1;
public:
  VoltageSource(double v, int p, int n);
  void setVoltage(double v);
  void stamp(Eigen::MatrixXd &matrix, Eigen::VectorXd &rhs, double currentTime, double dt) override;
};
