#include "CapacitorModel.hpp"
#include "../../core/Application.hpp"
#include "../Circuit.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <filesystem>

CapacitorModel::CapacitorModel(double C, int n1, int n2) : C(C), node1(n1), node2(n2) {
  SDL_Renderer *renderer =
      SDL_GetRenderer(Application::getInstance()->getWindow());
  std::filesystem::path p = std::filesystem::current_path().parent_path() /
                            "assets/icons/capacitor.png";
  texture_ = IMG_LoadTexture(renderer, p.c_str());
}

void CapacitorModel::stamp(Eigen::MatrixXd &G, Eigen::VectorXd &I,
                      double currentTime, double dt) {
  double Geq = C / dt;
  double Ieq = Geq * prevVoltage;

  if (!Circuit::isNodeGround(node1)) {
    G(node1, node1) += Geq;
    I(node1) -= Ieq;
    if (!Circuit::isNodeGround(node2)) {
      G(node1, node2) -= Geq;
      G(node2, node1) -= Geq;
    }
  }
  if (!Circuit::isNodeGround(node2)) {
    G(node2, node2) += Geq;
    I(node2) += Ieq;
  }
}

void CapacitorModel::updateState(const Eigen::VectorXd &V,
                            const Eigen::VectorXd &I) {
  double vNode1 = Circuit::isNodeGround(node1) ? 0.0 : V(node1);
  double vNode2 = Circuit::isNodeGround(node2) ? 0.0 : V(node2);
  prevVoltage = vNode2 - vNode1;
}

void CapacitorModel::initializeState() { prevVoltage = 0.0; }
