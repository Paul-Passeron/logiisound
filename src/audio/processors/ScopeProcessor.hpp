#pragma once
#include "Processor.hpp"
#include <imgui.h>
#include <vector>
#include <cstring>

class ScopeProcessor : public Processor {
    std::vector<float> displayBuffer; // Buffer to hold snapshot for visual display
    size_t bufferSize;
    size_t bufferIndex;

public:
    ScopeProcessor(size_t bufferSize = 2048);
    ~ScopeProcessor() override = default;

    void process(float **inputBuffer, float **outputBuffer, size_t numSamples) override;
    void render() override;
};
