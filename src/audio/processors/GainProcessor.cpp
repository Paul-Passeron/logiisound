#include "GainProcessor.hpp"
#include <imgui.h>

GainProcessor::GainProcessor() : Processor(), gain(1.0f) {}

void GainProcessor::render() {
  float gain = this->getGain();
  ImGui::Text("Gain Processor");
  ImGui::Text("Master Gain");
  ImGui::SameLine();
  ImGui::PushID(ImGuiHash);
  ImGui::SliderFloat("", &gain, 0.0f, 3.0f, "%.4f", ImGuiSliderFlags_Logarithmic);
  this->setGain(gain);
  ImGui::PopID();
}

void GainProcessor::setGain(float newGain) { this->gain = newGain; }

float GainProcessor::getGain() { return this->gain; }

void GainProcessor::process(float **inputBuffer, float **outputBuffer,
                            size_t numSamples) {
  for (size_t channel = 0; channel < this->numChannels; ++channel) {
    for (size_t sample = 0; sample < numSamples; ++sample) {
      outputBuffer[channel][sample] = getGain() * inputBuffer[channel][sample];
    }
  }
}
