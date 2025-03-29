#include "FilePlayer.hpp"
#include "Processor.hpp"
#include <filesystem>
#include <imgui.h>
#include <iostream>
#include <sndfile.h>
#include <tinyfiledialogs.h>

FilePlayer::FilePlayer() : Processor() {}

void FilePlayer::render() {
  ImGui::Text("File Player");
  ImGui::PushID(ImGuiHash);
  if (ImGui::Button("Browse##")) {
    this->onBrowsePressed();
  }
  ImGui::PopID();
  ImGui::PushID(ImGuiHash + 1);
  ImGui::Checkbox("Loop##", &this->loop);
  ImGui::PopID();
  ImGui::PushID(ImGuiHash + 2);
  string name = "";
  if (path.empty()) {
    name = "(None)";
  } else {
    name = std::filesystem::path(path).filename().c_str();
  }
  ImGui::Text("Current file: %s", name.c_str());
  ImGui::PopID();
}

bool FilePlayer::loadAudioFile(const string &filePath) {
  SNDFILE *file;
  SF_INFO sfinfo;
  file = sf_open(filePath.c_str(), SFM_READ, &sfinfo);
  if (!file) {
    std::cerr << "Failed to open file" << std::endl;
    return false;
  }

  std::cout << "Loaded file sample rate: " << sfinfo.samplerate << " Hz"
            << std::endl;

  if (sfinfo.samplerate != 44100) {
    std::cerr << "Sample rate mismatch" << std::endl;
    sf_close(file);
    return false;
  }

  audioData.resize(sfinfo.frames * sfinfo.channels);
  sf_read_float(file, &audioData[0], audioData.size());
  sf_close(file);
  path = filePath;
  playhead = 0;
  return true;
}

void FilePlayer::onBrowsePressed() {
  const char *filterPatterns[2] = {"*.wav", "*.aiff"};
  const char *filePath = tinyfd_openFileDialog("Select an audio file", "../assets/", 2,
                                               filterPatterns, nullptr, 0);

  if (filePath) {
    if (!loadAudioFile(filePath)) {
      tinyfd_messageBox("Error", "Failed to load audio file!", "ok", "error",
                        1);
    }
  }
}

void FilePlayer::process(float **inputBuffer, float **outputBuffer,
                         size_t numSamples) {
  if (audioData.empty()) {
    for (size_t i = 0; i < numChannels; ++i) {
      memset(outputBuffer[i], 0, numSamples);
    }
    return;
  }
  size_t sample;
  for (sample = 0; sample < numSamples; ++sample) {
    if (playhead >= audioData.size()) {
      if (loop) {
        playhead = 0;
      } else {
        break;
      }
    }
    for (size_t channel = 0; channel < numChannels; ++channel) {
      outputBuffer[channel][sample] = audioData[playhead++];
    }
  }
  if (sample != numSamples) {
    for (size_t i = 0; i < numChannels; ++i) {
      memset(outputBuffer[i] + sample - 1, 0, numSamples - sample);
    }
  }
}
