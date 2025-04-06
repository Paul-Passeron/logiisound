#include "ResistorModel.hpp"
#include "../Circuit.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdexcept>

ResistorModel::ResistorModel(double r, int n1, int n2)
    : resistance(r), node1(n1), node2(n2) {
  if (r <= 0) {
    throw std::runtime_error(
        "Can't have resistor with resistance <= 0. Replace with a connection.");
  }
}

void ResistorModel::stamp(Eigen::MatrixXd &matrix, Eigen::VectorXd &rhs,
                     double currentTime, double dt) {
  double G_ = 1.0 / resistance;

  if (!Circuit::isNodeGround(node1)) {
    matrix(node1, node1) += G_;
    if (!Circuit::isNodeGround(node2)) {
      matrix(node1, node2) -= G_;
      matrix(node2, node1) -= G_;
    }
  }
  if (!Circuit::isNodeGround(node2)) {
    matrix(node2, node2) += G_;
  }
}

void ResistorModel::fromJson(json data) {
  // TODO
}
