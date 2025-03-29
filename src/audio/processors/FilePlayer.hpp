#pragma once
#include "Processor.hpp"
#include <string>
#include <vector>

using std::string;
using std::vector;

class FilePlayer : public Processor {
  string path;
     bool loop;
     vector<float> audioData;
     size_t playhead = 0;

     void onBrowsePressed();
     bool loadAudioFile(const string& filePath);

 public:
     FilePlayer();
     virtual ~FilePlayer() override = default;
     void render() override;
     void process(float **inputBuffer, float **outputBuffer, size_t numSamples) override;
};
