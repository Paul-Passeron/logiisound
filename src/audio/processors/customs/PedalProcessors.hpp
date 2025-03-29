#include "../CircuitProcessor.hpp"

class PedalProcessors {
  public:
  static CircuitProcessor * FuzzProcessor();
  static CircuitProcessor * LowPassProcessor(double R, double C);
};
