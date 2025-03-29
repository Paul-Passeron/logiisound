#pragma once
#include "Processor.hpp"
#include <vector>

using std::vector;

class ChainProcessor : public Processor {
  vector<Processor *> processors;

public:
  ChainProcessor();
  ~ChainProcessor() override;
  void render() override;
  void process(float **inputBuffer, float **outputBuffer,
               size_t numSamples) override;
  void addProcessor(Processor *p);
};
