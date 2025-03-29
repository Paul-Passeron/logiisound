#include "SineGenerator.hpp"
#include <cmath>
#include <imgui.h>
#include <iostream>

void SineGenerator::render() {
  ImGui::Text("Sine Generator");
  ImGui::Text("Amplitude");
  ImGui::SameLine();
  ImGui::PushID(ImGuiHash);
  ImGui::SliderFloat("", &this->amplitude, 0.0f, 2.0f, "%.4f", ImGuiSliderFlags_Logarithmic);
  ImGui::Spacing();
  ImGui::Text("Frequency");
  ImGui::SameLine();
  ImGui::PushID(ImGuiHash + 1);
  ImGui::SliderFloat("", &this->frequency, 0.0f, 20000.0f, "%.2fHz",
                     ImGuiSliderFlags_Logarithmic);
  ImGui::PopID();
  ImGui::PopID();
}

void SineGenerator::process(float **inputBuffer, float **outputBuffer,
                            size_t numSamples) {
  for (size_t sample = 0; sample < numSamples; ++sample) {
    float value = amplitude * sinf(phase);
    outputBuffer[0][sample] = value;
    outputBuffer[1][sample] = value;
    phase += 2.0f * M_PI * frequency / sampleRate;
    if (phase >= 2 * M_PI) {
      phase -= 2 * M_PI;
    }
  }
}
SineGenerator::SineGenerator() : Processor() {}
