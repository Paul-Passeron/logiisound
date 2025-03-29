#include "SwitchProcessor.hpp"
#include "Processor.hpp"
#include <imgui.h>
#include <iostream>


SwitchProcessor::SwitchProcessor(Processor *p) : processor(p), on(false) {}

SwitchProcessor::~SwitchProcessor() { delete processor; }

void SwitchProcessor::render() {
  ImGui::Text("Switch Processor");
  processor->render();
  ImGui::Separator();
  ImGui::Checkbox("On", &on);
}

void SwitchProcessor::process(float **inputBuffer, float **outputBuffer,
                              size_t numSamples) {
  if (on) {
    processor->process(inputBuffer, outputBuffer, numSamples);
  } else {
    for (size_t channel = 0; channel < this->numChannels; ++channel) {
      if (inputBuffer != outputBuffer) {
        memcpy(outputBuffer[channel], inputBuffer[channel], sizeof(float) * numSamples);
      }
    }
  }
}
