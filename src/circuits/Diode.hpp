// #pragma once

// #include "Component.hpp"
// #include <eigen3/Eigen/Dense>

// class Diode : public Component {
//   int anode;
//   int cathode;
//   double Is = 1e-12; // Saturation current
//   double Vt = 0.26; // 26 mV
//   double currentVoltageGuess = 0.1;
//   double lastVd;
//   double lastId;
// public:
//   Diode(int anode, int cathode, double Is = 1e-12, double Vt = 0.26);
//   void stamp(Eigen::MatrixXd &G, Eigen::VectorXd &I, double t,
//              double dt) override;
//   void updateState(const Eigen::VectorXd &V, const Eigen::VectorXd &I) override;
// };

#pragma once

#include "Component.hpp"
#include <unordered_map>
#include <string>

struct DiodeModelParameters {
  double Is;  // Saturation current
  double N;   // Emission coefficient
  double Vj;  // Junction potential
  double M;   // Grading coefficient
  double Cj0; // Zero-bias junction capacitance
  double Tt;  // Transit time
};

class Diode : public Component {
private:
  int anode, cathode;
  double Vt;  // Thermal voltage

  DiodeModelParameters params;
  std::string model;

  double currentVoltage;
  double currentCurrent;
  double conductance;

  static std::unordered_map<std::string, DiodeModelParameters> modelLibrary;

  void stampCurrent(Eigen::MatrixXd &G, Eigen::VectorXd &I);

public:
  Diode(int anode, int cathode, const std::string &modelName);
  Diode(int anode, int cathode, const DiodeModelParameters &customParams);

  DiodeModelParameters getParameters() const;
  void setParameter(const std::string &param, double value);

  void stamp(Eigen::MatrixXd &G, Eigen::VectorXd &I, double t, double dt) override;
  void updateState(const Eigen::VectorXd &V, const Eigen::VectorXd &I) override;
  void initializeState() override;
};
