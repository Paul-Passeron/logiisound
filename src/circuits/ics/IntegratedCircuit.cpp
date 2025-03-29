#include "IntegratedCircuit.hpp"

void IntegratedCircuit::stamp(Eigen::MatrixXd &matrix, Eigen::VectorXd &rhs,
                              double currentTime, double dt) {
  internalG.setZero();
  internalI.setZero();
  c->stamp(internalG, internalI, currentTime, dt);
  size_t mySize = start + internalI.size();

  if (rhs.cols() < mySize) {
    rhs.conservativeResize(mySize);
  }
  if (matrix.cols() < mySize) {
    matrix.conservativeResize(mySize, mySize);
    matrix.col(mySize - 1).setZero();
    matrix.row(mySize - 1).setZero();
  }
  for (size_t i = start; i < mySize; ++i) {
    rhs(i) += internalI(i - start);
    for (size_t j = start; j < mySize; ++j) {
      matrix(i, j) += internalG(i - start, j - start);
      matrix(j, i) += internalG(j - start, i - start);
    }
  }

  // merging nodes

  for (auto connexion : connexions) {
    int external = connexion.first;
    int internal = connexion.second;
    for (size_t i = 0; i < matrix.cols(); ++i) {
      matrix(external, i) += internalG(internal, i);
      matrix(i, external) += internalG(i, internal);
    }
    rhs(external) += internalI(internal);

    rhs(internal + start) = 0;
    matrix.col(internal + start).setZero();
    matrix.row(internal + start).setZero();
  }
}

void IntegratedCircuit::updateState(const Eigen::VectorXd &V) {
  c->updateState(V);
}

void IntegratedCircuit::addConnexion(int extNode, int intNode) {
  connexions.emplace_back(extNode, intNode);
}
