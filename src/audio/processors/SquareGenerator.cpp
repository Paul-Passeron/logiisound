#include "SquareGenerator.hpp"
#include <cmath>
#include <imgui.h>

void SquareGenerator::render() {
  ImGui::Text("Square Generator");
  ImGui::Text("Amplitude");
  ImGui::SameLine();
  ImGui::PushID(ImGuiHash);
  ImGui::SliderFloat("", &this->amplitude, 0.0f, 2.0f, "%.2f");
  ImGui::Spacing();
  ImGui::Text("Frequency");
  ImGui::SameLine();
  ImGui::PushID(ImGuiHash + 1);
  ImGui::SliderFloat("", &this->frequency, 0.0f, 20000.0f, "%.2fHz",
                     ImGuiSliderFlags_Logarithmic);
  ImGui::PopID();
  ImGui::PopID();
}

double signf(double x) {
  return x > 0.0 ? 1.0 : -1.0;
}

void SquareGenerator::process(float **inputBuffer, float **outputBuffer,
                            size_t numSamples) {
  for (size_t sample = 0; sample < numSamples; ++sample) {
    float val = amplitude * signf(sinf(phase));
    for (size_t channel = 0; channel < this->numChannels; ++channel) {
      outputBuffer[channel][sample] = val;
    }
    phase += 2.0f * M_PI * frequency / sampleRate;
    if (phase >= 2 * M_PI) {
      phase -= 2 * M_PI;
    }
  }
}
SquareGenerator::SquareGenerator() : Processor() {}
