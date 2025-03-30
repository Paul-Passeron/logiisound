#include "Circuit.hpp"
#include "ComponentModel.hpp"
#include "VoltageSourceModel.hpp"
#include <eigen3/Eigen/Dense>
#include <eigen3/Eigen/src/Core/Matrix.h>
#include <iostream>
#include <stdexcept>

bool Circuit::isNodeGround(int node) { return node < 0; }

int Circuit::addComponent(ComponentModel *comp) {
  components.emplace_back(comp);
  int res = components.size() - 1;
  stamp(G, I, 0, 1); // stamping to update I and G sizes for getLastIndex
  return res;
}

Circuit::Circuit(int n) : numNodes(n) {
  G = Eigen::MatrixXd::Zero(numNodes, numNodes);
  I = Eigen::VectorXd::Zero(numNodes);
}

void Circuit::solveTransient(double start, double dt, size_t numSamples,
                             int inputNode, int outputL, int outputR,
                             float **inputBuffer, float **outputBuffer) {
  for (auto c : components) {
    // c->initializeState();
  }
  if (isNodeGround(outputL) || outputL >= numNodes || isNodeGround(outputR) ||
      outputR >= numNodes) {
    return;
  }

  double t = start;

  VoltageSourceModel *v = nullptr;
  if (components.size() > inputNode || inputNode < 0) {
    v = dynamic_cast<VoltageSourceModel *>(components[inputNode]);
  }
  if (!v) {
    throw std::runtime_error("Input is not a voltage source.\n");
  }
  Eigen::VectorXd V;

  for (size_t i = 0; i < numSamples; ++i) {
    // Initialize state from previous timestep
    Eigen::VectorXd V_prev = V; // Store previous solution

    const int MAX_ITERATIONS = 2;
    const double CONVERGENCE_THRESHOLD = 1e-5;
    bool converged = false;
    double error = 0;
    for (int iter = 0; iter < MAX_ITERATIONS && !converged; iter++) {
      G.setZero();
      I.setZero();
      v->setVoltage(inputBuffer[0][i]);
      stamp(G, I, t, dt);
      Eigen::VectorXd V_next = G.fullPivLu().solve(I);
      if (iter > 0) {
        error = (V_next - V_prev).norm() / V_next.norm();
        converged = (error < CONVERGENCE_THRESHOLD);
      }
      V_prev = V_next;
      updateState(V_next);
    }
    // if (!converged) {
    //   std::cout << "Did not converge... (" << error << ")" << std::endl;
    // }

    V = V_prev; // Final solution for this timestep
    outputBuffer[0][i] = V(outputL);
    outputBuffer[1][i] = V(outputR);

    t += dt;
  }
}

int Circuit::getNumStates() { return numNodes; }

void Circuit::stamp(Eigen::MatrixXd &outG, Eigen::VectorXd &outI, double t,
                    double dt) {
  for (auto comp : components) {
    comp->stamp(outG, outI, t, dt);
  }
}

void Circuit::updateState(const Eigen::VectorXd &V) {
  for (auto comp : components) {
    comp->updateState(V, I);
  }
}

int Circuit::getLastIndex() { return I.size(); }

void Circuit::initializeState() {
  for (auto comp : components) {
    comp->initializeState();
  }
}
