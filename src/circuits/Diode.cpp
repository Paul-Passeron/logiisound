// #include "Diode.hpp"
// #include "Circuit.hpp"
// #include <cmath>
// #include <iostream>

// const double kDefaultVT =
//     0.026; // Default Thermal Voltage at room temperature (26mV)
// const double kDefaultIdealityFactor = 1.0; // Common assumption for
// simplicity

// Diode::Diode(int anode, int cathode, double Is, double Vt)
//     : anode(anode), cathode(cathode), Is(Is), Vt(Vt) {}

// void Diode::stamp(Eigen::MatrixXd &G, Eigen::VectorXd &I, double t, double
// dt) {
//   if (!currentVoltageGuess) {
//     currentVoltageGuess = 0.1;
//   }
//   double n = kDefaultIdealityFactor;
//   double Vd = currentVoltageGuess;
//   double e = exp(Vd / (n * Vt));
//   double Id = Is * (e - 1);
//   double Gd = Is * e / (n * Vt);
//   // double Gd = (lastVd - Id) / (lastVd - Vd);
//   double Id0 = Id - Gd * Vd;

//   if (!Circuit::isNodeGround(anode) && !Circuit::isNodeGround(cathode)) {
//     G(anode, cathode) -= Gd;
//     G(cathode, anode) -= Gd;
//   }
//   if (!Circuit::isNodeGround(anode)) {
//     G(anode, anode) += Gd;
//     I(anode) -= Id0;
//   }
//   if (!Circuit::isNodeGround(cathode)) {
//     G(cathode, cathode) += Gd;
//     I(cathode) += Id0;
//   }
//   lastId = Id;
// }

// void Diode::updateState(const Eigen::VectorXd &V, const Eigen::VectorXd &I) {
//   double vNodeAnode = Circuit::isNodeGround(anode) ? 0.0 : V(anode);
//   double vNodeCathode = Circuit::isNodeGround(cathode) ? 0.0 : V(cathode);
//   lastVd = currentVoltageGuess;
//   currentVoltageGuess = vNodeAnode - vNodeCathode;
// }

#include "Diode.hpp"
#include "Circuit.hpp"
#include <cmath>
#include <stdexcept>

DiodeModelParameters oneN5817Parameters = {
    .Is = 1e-6, // Saturation current; higher for Schottky diodes
    .N = 1.2,   // Emission coefficient; slightly higher due to Schottky nature
    .Vj = 0.22, // Junction potential; lower than silicon junctions
    .M = 0.45,  // Grading coefficient; common assumption
    .Cj0 = 110e-12, // Zero-bias capacitance; high due to larger junction size
    .Tt = 0.0     // Transit time; typically negligible for Schottky diodes
};

DiodeModelParameters bat41Parameters = {
    .Is = 1e-7, // Saturation current
    .N = 1.05,  // Emission coefficient; around 1-1.1 for Schottky
    .Vj = 0.25, // Junction potential; typically lower than silicon
    .M = 0.45,  // Grading coefficient; assumed value
    .Cj0 = 80e-12,  // Zero-bias junction capacitance; medium for Schottky
    .Tt = 0.0   // Transit time; typically negligible
};

std::unordered_map<std::string, DiodeModelParameters> Diode::modelLibrary = {
    {"1N4148",
     {.Is = 2.52e-9,
      .N = 1.752,
      .Vj = 0.7,
      .M = 0.342,
      .Cj0 = 4e-12,
      .Tt = 0.0}},
    {"1N34A",
     {.Is = 2.52e-9, .N = 1.0, .Vj = 0.3, .M = 0.5, .Cj0 = 2e-12, .Tt = 0.0}},
    {"1N5817", oneN5817Parameters},
    {"BAT41", bat41Parameters}};

Diode::Diode(int anode, int cathode, const std::string &modelName)
    : anode(anode), cathode(cathode), Vt(0.026) {
  if (modelLibrary.find(modelName) == modelLibrary.end()) {
    throw std::runtime_error("Invalid diode model name: " + modelName);
  }
  params = modelLibrary[modelName];
  model = modelName;
  initializeState();
}

Diode::Diode(int anode, int cathode, const DiodeModelParameters &customParams)
    : anode(anode), cathode(cathode), Vt(0.026), params(customParams),
      model("Custom") {
  initializeState();
}

void Diode::initializeState() {
  currentVoltage = 0.0;
  currentCurrent = 0.0;
  conductance = 1e-9; // Tiny conductance to avoid division by zero
}

void Diode::stampCurrent(Eigen::MatrixXd &G, Eigen::VectorXd &I) {
  if (!Circuit::isNodeGround(anode)) {
    I(anode) -= currentCurrent;
    G(anode, anode) += conductance;
    if (!Circuit::isNodeGround(cathode)) {
      G(anode, cathode) -= conductance;
    }
  }
  if (!Circuit::isNodeGround(cathode)) {
    I(cathode) += currentCurrent;
    G(cathode, cathode) += conductance;
    if (!Circuit::isNodeGround(anode)) {
      G(cathode, anode) -= conductance;
    }
  }
}

void Diode::stamp(Eigen::MatrixXd &G, Eigen::VectorXd &I, double t, double dt) {
  stampCurrent(G, I);
}

void Diode::updateState(const Eigen::VectorXd &V, const Eigen::VectorXd &I) {
  double vAnode = Circuit::isNodeGround(anode) ? 0.0 : V(anode);
  double vCathode = Circuit::isNodeGround(cathode) ? 0.0 : V(cathode);

  currentVoltage = vAnode - vCathode;
  double exp_term = exp(currentVoltage / (params.N * Vt));

  currentCurrent = params.Is * (exp_term - 1);
  conductance = (params.Is / (params.N * Vt)) * exp_term;
}
