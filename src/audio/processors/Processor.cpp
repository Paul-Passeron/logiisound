#include "Processor.hpp"
#include <cstdlib>

Processor::Processor(float sampleRate, size_t numChannels)
    : sampleRate(sampleRate), numChannels(numChannels) {
  ImGuiHash = rand();
}

void Processor::prepare(float sampleRate, size_t numChannels) {
  this->sampleRate = sampleRate;
  this->numChannels = numChannels;
}

void Processor::reset() {}
