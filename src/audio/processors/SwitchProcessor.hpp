#pragma once
#include "Processor.hpp"
#include <vector>

using std::vector;

class SwitchProcessor : public Processor {
  Processor * processor;
  bool on;

public:
  SwitchProcessor(Processor *p);
  ~SwitchProcessor() override;
  void render() override;
  void process(float **inputBuffer, float **outputBuffer,
               size_t numSamples) override;
};
