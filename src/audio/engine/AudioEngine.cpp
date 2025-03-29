#include "AudioEngine.hpp"
#include <cmath>
#include <iostream>
#include <portaudio.h>
#include <stdexcept>

int AudioEngine::audioCallback(const void *inputBuffer, void *outputBuffer,
                               unsigned long framesPerBuffer,
                               const PaStreamCallbackTimeInfo *timeInfo,
                               PaStreamCallbackFlags flags, void *userData) {
  AudioEngine *engine = static_cast<AudioEngine *>(userData);
  float **out = (float **)outputBuffer;
  float **in = (float **)inputBuffer;
  engine->processor->process(in, out, framesPerBuffer);
  return paContinue;
}

AudioEngine::AudioEngine(Processor *proc) : processor(proc), stream(nullptr) {
  PaError err = Pa_Initialize();
  if (err != paNoError) {
    throw std::runtime_error("Failed to initialize PortAudio");
  }
  this->phase = 0;
}

AudioEngine::~AudioEngine() { Pa_Terminate(); }

void AudioEngine::start() {
  if (this->stream) {
    this->stop();
  }

  PaStreamParameters inputParameters;
  inputParameters.device = Pa_GetDefaultInputDevice();
  if (inputParameters.device == paNoDevice) {
    throw std::runtime_error("No default input device found");
  }
  inputParameters.channelCount = 2;
  inputParameters.sampleFormat = paFloat32 | paNonInterleaved;
  inputParameters.suggestedLatency =
      Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency;
  inputParameters.hostApiSpecificStreamInfo = nullptr;

  // Setup output parameters
  PaStreamParameters outputParameters;
  outputParameters.device = Pa_GetDefaultOutputDevice();
  if (outputParameters.device == paNoDevice) {
    throw std::runtime_error("No default output device found");
  }
  outputParameters.channelCount = 2;
  outputParameters.sampleFormat = paFloat32 | paNonInterleaved;
  outputParameters.suggestedLatency =
      Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
  outputParameters.hostApiSpecificStreamInfo = nullptr;

  unsigned long framesPerBuffer = 64;

  double sampleRate = 44100;
  // Open stream with both input and output
  PaError err =
      Pa_OpenStream(&stream, &inputParameters, &outputParameters, sampleRate,
                    framesPerBuffer, paClipOff, audioCallback, this);

  this->processor->prepare(sampleRate, 2);
  if (err != paNoError) {
    std::cerr << "Failed to open stream: " << Pa_GetErrorText(err) << std::endl;
    throw std::runtime_error("Failed to open stream with PortAudio");
  }

  err = Pa_StartStream(stream);
  if (err != paNoError) {
    std::cerr << "Failed to start stream: " << Pa_GetErrorText(err)
              << std::endl;
    Pa_CloseStream(stream);
    throw std::runtime_error("Failed to start PortAudio stream");
  }
  std::cout << "Stream started with input device " << inputParameters.device
            << " and output device " << outputParameters.device << std::endl;
}

void AudioEngine::stop() {
  if (this->stream) {
    Pa_StopStream(this->stream);
    Pa_CloseStream(this->stream);
    this->stream = nullptr;
  }
}

Processor *AudioEngine::getProcessor() { return processor; }
