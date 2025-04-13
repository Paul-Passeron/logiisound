#pragma once

#include "ComponentModel.hpp"
#include <string>
#include <unordered_map>

using std::string;

struct DiodeModelParameters {
  double Is;  // Saturation current
  double N;   // Emission coefficient
  double Vj;  // Junction potential
  double M;   // Grading coefficient
  double Cj0; // Zero-bias junction capacitance
  double Tt;  // Transit time
};

class DiodeModel : public ComponentModel {
protected:
  static vector<string> models;

private:
  int anode, cathode;
  double Vt; // Thermal voltage

  DiodeModelParameters params;
  std::string model;

  double currentVoltage;
  double currentCurrent;
  double conductance;

  static std::unordered_map<std::string, DiodeModelParameters> modelLibrary;

  void stampCurrent(Eigen::MatrixXd &G, Eigen::VectorXd &I);

public:
  DiodeModel(int anode, int cathode, const std::string &modelName);
  DiodeModel(int anode, int cathode, const DiodeModelParameters &customParams);

  DiodeModelParameters getParameters() const;
  void setParameter(const std::string &param, double value);
  void stamp(Eigen::MatrixXd &G, Eigen::VectorXd &I, double t,
             double dt) override;
  void updateState(const Eigen::VectorXd &V, const Eigen::VectorXd &I) override;
  void initializeState() override;
  static const vector<string> &getModels();
};
