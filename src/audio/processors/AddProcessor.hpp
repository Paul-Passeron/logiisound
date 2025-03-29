#pragma once
#include "Processor.hpp"

class AddProcessor : public Processor {
  Processor *a;
  Processor *b;
  float **buffer = nullptr;
  float mix = 0.5f;

public:
  AddProcessor(Processor *a, Processor *b);
  ~AddProcessor() override;
  void render() override;
  void process(float **inputBuffer, float **outputBuffer,
               size_t numSamples) override;
};
