#include "ComponentModel.hpp"

void ComponentModel::updateState(const Eigen::VectorXd &V,
                            const Eigen::VectorXd &I) {
  // Do nothing for non transient basic components...
}

void ComponentModel::initializeState() {
  // Do nothing for non transient basic components...
}
