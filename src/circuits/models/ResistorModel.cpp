#include "ResistorModel.hpp"
#include "../../core/Application.hpp"
#include "../Circuit.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <filesystem>
#include <iostream>
#include <stdexcept>

ResistorModel::ResistorModel(double r, int n1, int n2)
    : resistance(r), node1(n1), node2(n2) {
  SDL_Renderer *renderer =
      SDL_GetRenderer(Application::getInstance()->getWindow());
  std::filesystem::path p = std::filesystem::current_path().parent_path() /
                            "assets/icons/resistor.png";
  texture_ = IMG_LoadTexture(renderer, p.c_str());
  if (texture_ == nullptr) {
    std::cerr << "[ERROR]: Failed to load texture for Resistor ("
              << SDL_GetError() << "), " << renderer << std::endl;
  }
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
