#include "ScopeProcessor.hpp"
#include "imgui.h"
#include <cmath>
#include <cstring>

ScopeProcessor::ScopeProcessor(size_t bufferSize)
    : Processor(), bufferSize(bufferSize), bufferIndex(0), displayBuffer(bufferSize, 0.0f) {
}

void ScopeProcessor::process(float **inputBuffer, float **outputBuffer, size_t numSamples) {
    // Copy input to output to pass the audio through unchanged
    for (size_t channel = 0; channel < numChannels; ++channel) {
        std::memcpy(outputBuffer[channel], inputBuffer[channel], numSamples * sizeof(float));
    }

    // Update display buffer with rolling window
    for (size_t i = 0; i < numSamples; ++i) {
        displayBuffer[bufferIndex] = inputBuffer[0][i];
        bufferIndex = (bufferIndex + 1) % bufferSize;
    }
}

void ScopeProcessor::render() {
    // Display the rolling buffer with a Line plot
    std::vector<float> displayBufferTemp(bufferSize);
    size_t currentIndex = bufferIndex;
    for (size_t i = 0; i < bufferSize; ++i) {
        currentIndex = (bufferIndex + i) % bufferSize;
        displayBufferTemp[i] = displayBuffer[currentIndex];
    }

    ImGui::PushID(ImGuiHash);
    ImGui::PlotLines("##", displayBufferTemp.data(), bufferSize, 0, nullptr, -1.0f, 1.0f, ImVec2(0, 150));
    ImGui::PopID();
}
