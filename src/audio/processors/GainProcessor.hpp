#pragma once
#include "Processor.hpp"

class GainProcessor : public Processor {
  float gain;

public:
  GainProcessor();
  ~GainProcessor() override = default;
  void render() override;
  void process(float **inputBuffer, float **outputBuffer,
               size_t numSamples) override;
  void setGain(float gain);
  float getGain();


};
