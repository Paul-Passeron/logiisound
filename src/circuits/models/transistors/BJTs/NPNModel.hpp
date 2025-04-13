#pragma once

#include "../../ComponentModel.hpp"

using std::string;

struct NPNModelParameters {
  double Is;   // Sat. current
  double Bf;   // Forward beta
  double Br;   // Reverse beta
  double Vaf;  // Forward early voltage
  double Var;  // Reverse early voltage
  double Rb;   // Base resistance
  double Re;   // Emitter resistance
  double Rc;   // Collector resistance
  double Cje;  // Base-emitter capacitance
  double Cjc;  // Base-collector capacitance
  double Tf;   // Forward transit time
  double Tr;   // Reverse transit time
  double Xcjc; // Fraction of Cjc connected to internal base
  double Vtf;  // Transit time dependance on Vbc
};

class NPNModel : public ComponentModel {
private:
  static vector<string> models;

  int b, c, e; // Base, Collector, Emitter nodes
  double Vt;   // Thermal voltage (≈26mV at room temperature)
  double Is;   // Saturation current

  NPNModelParameters params;
  std::string model;

  // State variables
  double Ic;  // Collector current
  double Ib;  // Base current
  double Ie;  // Emitter current
  double Vbe; // Base-emitter voltage
  double Vce; // Collector-emitter voltage
  double Vbc; // Base-collector voltage

  double IBE_eq, IBC_eq, ICE_eq;
  double g_0, g_mu, g_pi, g_m;

  // Derivatives used for linearization
  double dIc_dVbe; // Transconductance: change in Ic with Vbe
  double dIc_dVbc; // Change in Ic with Vbc
  double dIb_dVbe; // Change in Ib with Vbe
  double dIb_dVbc; // Change in Ib with Vbc

  // Model library
  static std::unordered_map<std::string, NPNModelParameters> modelLibrary;

  void stampBaseCurrent(Eigen::MatrixXd &G, Eigen::VectorXd &I);
  void stampCollectorCurrent(Eigen::MatrixXd &G, Eigen::VectorXd &I);
  void stampEmitterCurrent(Eigen::MatrixXd &G, Eigen::VectorXd &I);

public:
  NPNModel(int b, int c, int e,
           const std::string &modelName);     // Use predefined model
  NPNModel(int b, int c, int e, double beta); // Simple model with custom beta
  NPNModel(int b, int c, int e,
           const NPNModelParameters &customParams); // Fully custom model

  NPNModelParameters getParameters() const;
  void setParameter(const std::string &param, double value);

  void stamp(Eigen::MatrixXd &G, Eigen::VectorXd &I, double t,
             double dt) override;

  void updateState(const Eigen::VectorXd &V, const Eigen::VectorXd &I) override;

  void initializeState() override;
  static const vector<string> &getModels();
};
