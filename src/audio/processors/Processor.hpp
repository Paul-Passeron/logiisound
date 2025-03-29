#pragma once

// Base class for audio processors
#include <cstddef>
class Processor {

protected:
  float sampleRate;
  size_t numChannels;
  int ImGuiHash = 0;

public:
  Processor(float sampleRate = 44100.0f, size_t numChannels = 2);
  virtual ~Processor() = default;
  virtual void process(float **inputBuffer, float **outputBuffer, size_t numSamples) = 0;
  virtual void prepare(float sampleRate = 44100.0f, size_t numChannels = 2);
  virtual void reset();
  virtual void render() = 0;
};
