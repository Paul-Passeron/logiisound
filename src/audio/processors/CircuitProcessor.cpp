#include "CircuitProcessor.hpp"
#include <imgui.h>

CircuitProcessor::CircuitProcessor(Circuit *c) : Processor(), circuit(c) {}

CircuitProcessor::~CircuitProcessor() { delete circuit; }

void CircuitProcessor::render() {
  // TODO
  ImGui::Text("Circuit Processor");
}

void CircuitProcessor::process(float **inputBuffer, float **outputBuffer,
                               size_t numSamples) {

  circuit->solveTransient(time, 1.0 / sampleRate, numSamples, inputNode,
                          outputNode, outputNode, inputBuffer, outputBuffer);
  time += (double)numSamples / sampleRate;
}

void CircuitProcessor::setInput(int node){
  inputNode = node;
}

void CircuitProcessor::setOutput(int node) {
  outputNode = node;
}
