#include "AddProcessor.hpp"
#include <cmath>
#include <cstdlib>
#include <imgui.h>

AddProcessor::AddProcessor(Processor *a, Processor *b) : Processor() {
  this->a = a;
  this->b = b;
}

AddProcessor::~AddProcessor() {
  delete a;
  delete b;
  for (size_t channel = 0; channel < numChannels; ++channel) {
    free(buffer[channel]);
  }
  free(buffer);
}

void AddProcessor::render() {
  if (!ImGui::BeginChild("addproc", ImVec2(0, 0),
                        ImGuiChildFlags_AutoResizeX |
                            ImGuiChildFlags_AutoResizeY |
                            ImGuiChildFlags_Borders)) {
    ImGui::EndChild();
    return;
  }
  ImGui::Text("Add Processor");
  ImGui::BeginTable("addtable", 2, ImGuiTableFlags_BordersV);
  ImGui::TableNextColumn();
  a->render();
  ImGui::TableNextColumn();
  b->render();
  ImGui::EndTable();
  ImGui::Separator();
  ImGui::Text("Mix");
  ImGui::SameLine();
  ImGui::SliderFloat("##addprocslider", &this->mix, 0.0f, 1.0f, "%.2f");
  ImGui::EndChild();
}

void AddProcessor::process(float **inputBuffer, float **outputBuffer,
                           size_t numSamples) {
  if (this->buffer == nullptr) {
    this->buffer = (float **)malloc(numChannels * sizeof(float *));
    for (size_t channel = 0; channel < numChannels; channel++) {
      this->buffer[channel] = (float *)malloc(numSamples * sizeof(float));
      memcpy(this->buffer[channel], inputBuffer[channel], numSamples);
      memcpy(outputBuffer[channel], inputBuffer[channel], numSamples);
    }
  }
  float oneminus = 1.0 - this->mix;
  float common = 1.0 / sqrtf(this->mix * this->mix + oneminus * oneminus);
  this->a->process(outputBuffer, outputBuffer, numSamples);
  this->b->process(buffer, buffer, numSamples);
  for (size_t channel = 0; channel < numChannels; channel++) {
    for (size_t sample = 0; sample < numSamples; sample++) {
      float valueA = outputBuffer[channel][sample];
      float valueB = buffer[channel][sample];
      outputBuffer[channel][sample] =
          common * (mix * valueB + oneminus * valueA);
    }
  }
}
