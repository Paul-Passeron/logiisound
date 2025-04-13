#include "NPNModel.hpp"
#include "../../../Circuit.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdexcept>

std::unordered_map<std::string, NPNModelParameters> NPNModel::modelLibrary = {
    {"BC337",
     {
         .Is = 5.09e-14,  // Saturation current (A)
         .Bf = 180.0,     // Forward current gain (hFE)
         .Br = 4.0,       // Reverse current gain
         .Vaf = 74.0,     // Forward Early voltage (V)
         .Var = 20.0,     // Reverse Early voltage (V)
         .Rb = 10.0,      // Base resistance (Ω)
         .Re = 0.5,       // Emitter resistance (Ω)
         .Rc = 10.0,      // Collector resistance (Ω)
         .Cje = 11.5e-12, // Base-emitter capacitance (F)
         .Cjc = 5.5e-12,  // Base-collector capacitance (F)
         .Tf = 0.3e-9,    // Forward transit time (s)
         .Tr = 10.0e-9,   // Reverse transit time (s)
         .Xcjc = 1.0,     // Fraction of Cjc connected to internal base
         .Vtf = 1.7       // Transit time dependence on Vbc
     }},
    {"2N3904",
     {
         .Is = 6.734e-15, // Saturation current (A)
         .Bf = 150.0,     // Forward current gain (hFE)
         .Br = 3.0,       // Reverse current gain
         .Vaf = 74.0,     // Forward Early voltage (V)
         .Var = 24.0,     // Reverse Early voltage (V)
         .Rb = 10.0,      // Base resistance (Ω)
         .Re = 0.5,       // Emitter resistance (Ω)
         .Rc = 10.0,      // Collector resistance (Ω)
         .Cje = 4.5e-12,  // Base-emitter capacitance (F)
         .Cjc = 3.5e-12,  // Base-collector capacitance (F)
         .Tf = 0.4e-9,    // Forward transit time (s)
         .Tr = 100e-9,    // Reverse transit time (s)
         .Xcjc = 1.0,     // Fraction of Cjc connected to internal base
         .Vtf = 1.7       // Transit time dependence on Vbc
     }},
    {"2N2222",
     {
         .Is = 1.0e-14,  // Saturation current (A)
         .Bf = 200.0,    // Forward current gain (hFE)
         .Br = 5.0,      // Reverse current gain
         .Vaf = 100.0,   // Forward Early voltage (V)
         .Var = 30.0,    // Reverse Early voltage (V)
         .Rb = 10.0,     // Base resistance (Ω)
         .Re = 0.4,      // Emitter resistance (Ω)
         .Rc = 10.0,     // Collector resistance (Ω)
         .Cje = 7.5e-12, // Base-emitter capacitance (F)
         .Cjc = 5.0e-12, // Base-collector capacitance (F)
         .Tf = 0.3e-9,   // Forward transit time (s)
         .Tr = 60.0e-9,  // Reverse transit time (s)
         .Xcjc = 1.0,    // Fraction of Cjc connected to internal base
         .Vtf = 1.7      // Transit time dependence on Vbc
     }}};

NPNModel::NPNModel(int b, int c, int e, const std::string &modelName)
    : b(b), c(c), e(e), Vt(0.026) {
  if (modelLibrary.find(modelName) == modelLibrary.end()) {
    throw std::runtime_error("Bad model name " + modelName + ".");
  }
  // found model with the right name
  params = modelLibrary[modelName];
  model = modelName;
  initializeState();
}

NPNModel::NPNModel(int b, int c, int e, double beta)
    : NPNModel(b, c, e, "BC337") {
  params.Bf = beta;
  model = "Custom";
}

NPNModel::NPNModel(int b, int c, int e, const NPNModelParameters &customParams)
    : b(b), c(c), e(e), Vt(0.026), params(customParams), model("Custom") {
  initializeState();
}

void NPNModel::initializeState() {
  if (models.size() != modelLibrary.size()) {
    models.clear();
    models.resize(modelLibrary.size());
    int i = 0;
    for (const auto &[k, _] : modelLibrary) {
      models[i++] = k;
    }
  }

  // Reset all state variables
  Ic = 0.0;
  Ib = 0.0;
  Ie = 0.0;
  Vbe = 0.0;
  Vbc = 0.0;

  // Reset derivatives
  dIc_dVbe = 1e-9; // Small non-zero value to ensure matrix is not singular
  dIc_dVbc = 1e-9;
  dIb_dVbe = 1e-9;
  dIb_dVbc = 1e-9;
}

void NPNModel::stampBaseCurrent(Eigen::MatrixXd &G, Eigen::VectorXd &I) {
  if (!Circuit::isNodeGround(b)) {
    I(b) -= IBE_eq + IBC_eq;
    G(b, b) += g_mu + g_pi;
    if (!Circuit::isNodeGround(e)) {
      G(b, e) -= g_pi;
    }
    if (!Circuit::isNodeGround(c)) {
      G(b, c) -= g_mu;
    }
  }
}

void NPNModel::stampCollectorCurrent(Eigen::MatrixXd &G, Eigen::VectorXd &I) {
  if (!Circuit::isNodeGround(c)) {
    I(c) += IBC_eq - ICE_eq;
    G(c, c) += g_0 + g_mu;
    if (!Circuit::isNodeGround(b)) {
      G(c, b) += -g_mu + g_m;
    }
    if (!Circuit::isNodeGround(e)) {
      G(c, e) -= g_0 + g_m;
    }
  }
}

void NPNModel::stampEmitterCurrent(Eigen::MatrixXd &G, Eigen::VectorXd &I) {
  if (!Circuit::isNodeGround(e)) {
    I(e) += IBE_eq + ICE_eq;
    G(e, e) += g_pi + g_0 + g_m;
    if (!Circuit::isNodeGround(b)) {
      G(e, b) -= g_pi + g_m;
    }
    if (!Circuit::isNodeGround(c)) {
      G(e, c) -= g_0;
    }
  }
}

void NPNModel::stamp(Eigen::MatrixXd &G, Eigen::VectorXd &I, double t,
                     double dt) {
  stampBaseCurrent(G, I);
  stampCollectorCurrent(G, I);
  stampEmitterCurrent(G, I);
}

void NPNModel::updateState(const Eigen::VectorXd &V, const Eigen::VectorXd &I) {
  // Get node voltages
  double Vb = Circuit::isNodeGround(b) ? 0.0 : V(b);
  double Vc = Circuit::isNodeGround(c) ? 0.0 : V(c);
  double Ve = Circuit::isNodeGround(e) ? 0.0 : V(e);

  // Calculate voltage differences
  Vbe = Vb - Ve;
  Vbc = Vb - Vc;
  Vce = Vc - Ve;

  // Clamp voltage differences to prevent numerical overflow
  const double MAX_VBE =
      5.0 * 0.9; // Reasonable maximum for base-emitter voltage
  const double MAX_VBC =
      5.0 * 0.8; // Reasonable maximum for base-collector voltage

  // Apply clamping to prevent exponential overflow
  double Vbe_clamped = std::min(std::max(Vbe, -5.0 * 80.0 * Vt), MAX_VBE);
  double Vbc_clamped = std::min(std::max(Vbc, -5.0 * 80.0 * Vt), MAX_VBC);

  // Constants for model calculations
  double Ise = 0.0; // Default value for the moment
  double Isc = 0.0;
  double Nf = 1.0;
  double Ne = 1.5;
  double Nr = 1.0;
  double Nc = 2.0;

  // Calculate exponentials safely
  double exp_Vbe_Vt = std::exp(Vbe_clamped / Vt);
  double exp_Vbc_Vt = std::exp(Vbc_clamped / Vt);
  double exp_Vbe_NfVt = std::exp(Vbe_clamped / (Nf * Vt));
  double exp_Vbe_NeVt = std::exp(Vbe_clamped / (Ne * Vt));
  double exp_Vbc_NrVt = std::exp(Vbc_clamped / (Nr * Vt));
  double exp_Vbc_NcVt = std::exp(Vbc_clamped / (Nc * Vt));

  // Forward and reverse currents with Early effect
  double Vaf_clamped = std::max(params.Vaf, 10.0); // Prevent division by zero
  double Var_clamped = std::max(params.Var, 10.0);

  double If = params.Is * (exp_Vbe_Vt - 1.0) * (1.0 + Vce / Vaf_clamped);
  double Ir = params.Is * (exp_Vbc_Vt - 1.0) * (1.0 - Vce / Var_clamped);

  // Calculate conductances
  g_0 = params.Is * exp_Vbc_Vt / Vt;
  g_m = params.Is * exp_Vbe_Vt / Vt + g_0;

  g_pi = params.Is / (Nf * Vt * params.Bf) * exp_Vbe_NfVt +
         Ise / (Ne * Vt) * exp_Vbe_NeVt;
  g_mu = params.Is / (Nr * Vt * params.Br) * exp_Vbc_NrVt +
         Isc / (Nc * Vt) * exp_Vbc_NcVt;

  // Calculate base-emitter current components
  double Ibe_i = If / std::max(params.Bf, 1e-10); // Prevent division by zero
  double Ibe_n = Ise * (exp_Vbe_NeVt - 1.0);
  double Ibe = Ibe_i + Ibe_n;

  // Calculate base-collector current components
  double Ibc_i = Ir / std::max(params.Br, 1e-10); // Prevent division by zero
  double Ibc_n = Isc * (exp_Vbc_NcVt - 1.0);
  double Ibc = Ibc_i + Ibc_n;

  // Total current
  double I_t = params.Is * (exp_Vbe_Vt + exp_Vbc_Vt);

  // Calculate equivalent sources for nodal analysis
  IBE_eq = Ibe - g_pi * Vbe_clamped;
  IBC_eq =
      Ibc -
      g_mu * Vbc_clamped; // Fixed a probable bug here (was Ibc * g_mu * Vbc)
  ICE_eq = I_t - g_m * Vbe_clamped + g_0 * Vce;
}

vector<string> NPNModel::models;

const vector<string> &NPNModel::getModels() { return models; }
