#pragma once

#include "ComponentModel.hpp"

class VoltageSourceModel : public ComponentModel {
protected:
  double voltage;
  int posNode, negNode;
  bool called = false;
  int index = -1;
public:
  VoltageSourceModel(double v, int p, int n);
  void setVoltage(double v);
  void stamp(Eigen::MatrixXd &matrix, Eigen::VectorXd &rhs, double currentTime, double dt) override;
};
