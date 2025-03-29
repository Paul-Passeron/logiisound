#include "ChainProcessor.hpp"
#include "Processor.hpp"
#include <imgui.h>

void ChainProcessor::render() {
  bool first = true;
  for (Processor *p : this->processors) {
    if (!first) {
      ImGui::Separator();
    } else {
      first = false;
    }
    p->render();
  }
}

void ChainProcessor::process(float **inputBuffer, float **outputBuffer,
                             size_t numSamples) {

  if (inputBuffer != outputBuffer) {
    memcpy(outputBuffer[0], inputBuffer[0], sizeof(float) * numSamples);
    memcpy(outputBuffer[1], inputBuffer[1], sizeof(float) * numSamples);
  }
  for (Processor *p : this->processors) {
    p->process(outputBuffer, outputBuffer, numSamples);
  }
}

ChainProcessor::~ChainProcessor() {
  for (Processor *p : this->processors) {
    delete p;
  }
}

void ChainProcessor::addProcessor(Processor *p) {
  this->processors.emplace_back(p);
}

ChainProcessor::ChainProcessor() : Processor() {}
