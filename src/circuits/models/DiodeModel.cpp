#include "DiodeModel.hpp"
#include "../Circuit.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <cmath>
#include <stdexcept>

DiodeModelParameters bat46Parameters = {.Is = 3e-7, // Higher saturation current
                                        .N = 1.08,  // Emission coefficient
                                        .Vj = 0.7, // Higher junction potential
                                        .M = 0.45,
                                        .Cj0 = 85e-12,
                                        .Tt = 0.0};

DiodeModelParameters oneN5817Parameters = {
    .Is = 1e-6, // Saturation current; higher for Schottky diodes
    .N = 1.2,   // Emission coefficient; slightly higher due to Schottky nature
    .Vj = 0.22, // Junction potential; lower than silicon junctions
    .M = 0.45,  // Grading coefficient; common assumption
    .Cj0 = 110e-12, // Zero-bias capacitance; high due to larger junction size
    .Tt = 0.0       // Transit time; typically negligible for Schottky diodes
};

DiodeModelParameters bat41Parameters = {
    .Is = 1e-7,    // Saturation current
    .N = 1.05,     // Emission coefficient; around 1-1.1 for Schottky
    .Vj = 0.25,    // Junction potential; typically lower than silicon
    .M = 0.45,     // Grading coefficient; assumed value
    .Cj0 = 80e-12, // Zero-bias junction capacitance; medium for Schottky
    .Tt = 0.0      // Transit time; typically negligible
};

std::unordered_map<std::string, DiodeModelParameters> DiodeModel::modelLibrary =
    {
        {"1N4148",
         {.Is = 2.52e-9,
          .N = 1.752,
          .Vj = 0.7,
          .M = 0.342,
          .Cj0 = 4e-12,
          .Tt = 0.0}},
        {"1N34A",
         {.Is = 2.52e-9,
          .N = 1.0,
          .Vj = 0.3,
          .M = 0.5,
          .Cj0 = 2e-12,
          .Tt = 0.0}},
        {"1N5817", oneN5817Parameters},
        {"BAT41", bat41Parameters},
        {"BAT46", bat46Parameters},
};

DiodeModel::DiodeModel(int anode, int cathode, const std::string &modelName)
    : anode(anode), cathode(cathode), Vt(0.026) {
  if (modelLibrary.find(modelName) == modelLibrary.end()) {
    throw std::runtime_error("Invalid diode model name: " + modelName);
  }
  params = modelLibrary[modelName];
  model = modelName;
  initializeState();
}

DiodeModel::DiodeModel(int anode, int cathode,
                       const DiodeModelParameters &customParams)
    : anode(anode), cathode(cathode), Vt(0.026), params(customParams),
      model("Custom") {
  initializeState();
}

void DiodeModel::initializeState() {
  if (modelLibrary.size() != models.size()) {
    if (!models.empty()) {
      models.clear();
    }
    models.resize(modelLibrary.size());
    int i = 0;
    for (const auto [k, _] : modelLibrary) {
      models[i++] = k;
    }
  }
  currentVoltage = 0.0;
  currentCurrent = 0.0;
  conductance = 1e-9; // Tiny conductance to avoid division by zero
}

void DiodeModel::stampCurrent(Eigen::MatrixXd &G, Eigen::VectorXd &I) {
  if (!Circuit::isNodeGround(anode)) {
    I(anode) -= equivalentCurrent; // Use equivalentCurrent instead
    G(anode, anode) += conductance;
    if (!Circuit::isNodeGround(cathode)) {
      G(anode, cathode) -= conductance;
    }
  }
  if (!Circuit::isNodeGround(cathode)) {
    I(cathode) += equivalentCurrent;
    G(cathode, cathode) += conductance;
    if (!Circuit::isNodeGround(anode)) {
      G(cathode, anode) -= conductance;
    }
  }
}

void DiodeModel::stamp(Eigen::MatrixXd &G, Eigen::VectorXd &I, double t,
                       double dt) {
  stampCurrent(G, I);
}

// void DiodeModel::updateState(const Eigen::VectorXd &V,
//                              const Eigen::VectorXd &I) {
//   double vAnode = Circuit::isNodeGround(anode) ? 0.0 : V(anode);
//   double vCathode = Circuit::isNodeGround(cathode) ? 0.0 : V(cathode);

//   currentVoltage = vAnode - vCathode;
//   double exp_term = exp(currentVoltage / (params.N * Vt));

//   currentCurrent = params.Is * (exp_term - 1);
//   conductance = (params.Is / (params.N * Vt)) * exp_term;
// }
//
void DiodeModel::updateState(const Eigen::VectorXd &V,
                             const Eigen::VectorXd &I) {
  double vAnode = Circuit::isNodeGround(anode) ? 0.0 : V(anode);
  double vCathode = Circuit::isNodeGround(cathode) ? 0.0 : V(cathode);

  currentVoltage = vAnode - vCathode;
  double exp_term = exp(currentVoltage / (params.N * Vt));

  currentCurrent = params.Is * (exp_term - 1);
  conductance = (params.Is / (params.N * Vt)) * exp_term;

  // Calculate the equivalent current source
  equivalentCurrent = currentCurrent - conductance * currentVoltage;
}

const vector<string> &DiodeModel::getModels() { return models; }

vector<string> DiodeModel::models;
