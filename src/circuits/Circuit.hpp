#pragma once

#include "Component.hpp"
#include <eigen3/Eigen/Sparse>

class Circuit {
  vector<Component *> components;
  int numNodes;
  vector<int> data;
  Eigen::MatrixXd G;
  Eigen::VectorXd I;

public:
  Circuit(int nodes);
  int addComponent(Component *comp);
  void solveTransient(double start, double dt, size_t numSamples, int inputNode,
                      int outputL, int outputR, float **inputBuffer,
                      float **outputBuffer);
  int getNumStates();
  static bool isNodeGround(int node);
  void stamp(Eigen::MatrixXd &outG, Eigen::VectorXd &outI, double t, double dt);
  void updateState(const Eigen::VectorXd &V);
  int getLastIndex();
  void initializeState();
};
