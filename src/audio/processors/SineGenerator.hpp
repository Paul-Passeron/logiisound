#pragma once
#include "Processor.hpp"

class SineGenerator : public Processor {
  float amplitude = 0.5f;
  float frequency = 440.0f;
  float phase = 0.0f;

public:
  SineGenerator();
  ~SineGenerator() override = default;
  void render() override;
  void process(float **inputBuffer, float **outputBuffer,
               size_t numSamples) override;
};
