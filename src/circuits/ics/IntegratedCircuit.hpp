#include "../Circuit.hpp"
#include "../Component.hpp"
#include "eigen3/Eigen/Dense"

class IntegratedCircuit : public Component {
  Circuit *c;
  Eigen::MatrixXd internalG;
  Eigen::VectorXd internalI;
  int start; // start index
  // TODO: have a way to add connexions with the outside circuit, merging nodes together and effectively changing the matrix dimensions.
  std::vector<std::pair<int, int>> connexions;
public:
  IntegratedCircuit(Circuit *c, int start_index);
  void stamp(Eigen::MatrixXd &matrix, Eigen::VectorXd &rhs, double currentTime,
             double dt) override;
  void updateState(const Eigen::VectorXd &V) override;
  void addConnexion(int extNode, int intNode);
};
