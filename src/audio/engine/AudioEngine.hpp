#pragma once

#include "../processors/Processor.hpp"
#include <portaudio.h>
#include <vector>

using std::vector;

class AudioEngine {
  PaStream *stream;
  Processor *processor;
  int channels = 2;
  int sampleRate = 44100;
  float phase;



  static int audioCallback(const void *inputBuffer, void *outputBuffer,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags flags, void *userData);

public:
  AudioEngine(Processor *proc);
  ~AudioEngine();
  void start();
  void stop();
  Processor *getProcessor();
};
