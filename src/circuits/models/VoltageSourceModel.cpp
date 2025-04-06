#include "VoltageSourceModel.hpp"
#include "../Circuit.hpp"

VoltageSourceModel::VoltageSourceModel(double v, int p, int n)
    : voltage(v), posNode(p), negNode(n), called(false), index(-1) {}

void VoltageSourceModel::stamp(Eigen::MatrixXd &matrix, Eigen::VectorXd &rhs,
                          double currentTime, double dt) {

  int size = matrix.cols();
  if (!called) {
    index = size;
    called = true;
    matrix.conservativeResize(size + 1, size + 1);
    rhs.conservativeResize(size + 1);
    matrix.row(size).setZero();
    matrix.col(size).setZero();
  } else {
    size = index;
  }
  matrix(size, size) = 1.0;
  if (!Circuit::isNodeGround(posNode)) {
    matrix(size, posNode) = 1.0;
    matrix(posNode, size) = 1.0;
  }
  if (!Circuit::isNodeGround(negNode)) {
    matrix(size, negNode) = -1.0;
    matrix(negNode, size) = -1.0;
  }
  rhs(size) = voltage;
}

void VoltageSourceModel::setVoltage(double v) { voltage = v; }

void VoltageSourceModel::fromJson(json data) {
  // TODO
}
