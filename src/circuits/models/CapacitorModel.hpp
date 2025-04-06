#pragma once

#include "ComponentModel.hpp"

class CapacitorModel : public ComponentModel {
  double C;
  double prevVoltage;
  int node1, node2;

public:
  CapacitorModel(double C, int n1, int n2);
  void stamp(Eigen::MatrixXd &G, Eigen::VectorXd &I, double currentTime,
             double dt) override;
  void updateState(const Eigen::VectorXd &V, const Eigen::VectorXd &I) override;
  void initializeState() override;
  void fromJson(json data) override;

};
