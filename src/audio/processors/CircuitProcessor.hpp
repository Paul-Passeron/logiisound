#pragma once

#include "../../circuits/Circuit.hpp"
#include "Processor.hpp"

class CircuitProcessor : public Processor {
  Circuit *circuit;
  double time;
  int outputNode;
  int inputNode;

public:
  CircuitProcessor(Circuit *c);
  ~CircuitProcessor();
  void render() override;
  void process(float **inputBuffer, float **outputBuffer,
               size_t numSamples) override;
  Circuit *getCircuit();
  void setCircuit(Circuit *c);
  void setInput(int node);
  void setOutput(int node);

};
