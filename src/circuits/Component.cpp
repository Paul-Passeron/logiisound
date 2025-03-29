#include "Component.hpp"

void Component::updateState(const Eigen::VectorXd &V, const Eigen::VectorXd &I) {
  // Do nothing for non transient basic components...
}

void Component::initializeState() {
  // Do nothing for non transient basic components...
}
