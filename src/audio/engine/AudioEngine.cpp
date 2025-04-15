#include "AudioEngine.hpp"
#include <cmath>
#include <iostream>
#include <portaudio.h>
#include <stdexcept>

AudioEngine::AudioEngine(Processor *proc, int inputChannelStart)
    : processor(proc), stream(nullptr), inputChannelStart(inputChannelStart) {
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

  // Set the correct input channels (2 channels starting at inputChannelStart)
  inputParameters.channelCount = 2;
  inputParameters.sampleFormat = paFloat32 | paNonInterleaved;

  // Get device info to adjust parameters based on actual capabilities
  const PaDeviceInfo *deviceInfo = Pa_GetDeviceInfo(inputParameters.device);
  if (!deviceInfo) {
    throw std::runtime_error("Failed to get device info");
  }

  // Check if the requested channels are available
  if (inputChannelStart + 2 > deviceInfo->maxInputChannels) {
    std::cerr << "Warning: Device only has " << deviceInfo->maxInputChannels
              << " input channels, but channels " << inputChannelStart << "-"
              << (inputChannelStart + 1) << " were requested." << std::endl;

    // Fall back to default channels if requested ones are not available
    inputChannelStart = 0;
    std::cerr << "Falling back to channels 0-1" << std::endl;
  }

  std::cout << "Using input channels " << inputChannelStart << "-"
            << (inputChannelStart + 1) << std::endl;

  inputParameters.suggestedLatency =
      Pa_GetDeviceInfo(inputParameters.device)->defaultHighInputLatency;
  inputParameters.hostApiSpecificStreamInfo = nullptr;

  // Setup output parameters
  PaStreamParameters outputParameters;
  outputParameters.device = Pa_GetDefaultOutputDevice();
  if (outputParameters.device == paNoDevice) {
    throw std::runtime_error("No default output device found");
  }
  outputParameters.channelCount = 2;
  outputParameters.sampleFormat = paFloat32 | paNonInterleaved;

  // Increase output latency significantly
  outputParameters.suggestedLatency =
      Pa_GetDeviceInfo(outputParameters.device)->defaultHighOutputLatency;
  // Or set an explicit high value:
  // outputParameters.suggestedLatency = 0.1; // 100ms latency

  outputParameters.hostApiSpecificStreamInfo = nullptr;

  // Use a larger buffer size as well
  unsigned long framesPerBuffer = 512; // Increased from 64 to 512

  double sampleRate = 44100;

  // Prepare processor before opening stream
  this->processor->prepare(sampleRate, 2);

  std::cout << "Opening stream with input latency: "
            << inputParameters.suggestedLatency
            << "s, output latency: " << outputParameters.suggestedLatency
            << "s, buffer size: " << framesPerBuffer << std::endl;

  // Open stream with both input and output
  PaError err =
      Pa_OpenStream(&stream, &inputParameters, &outputParameters, sampleRate,
                    framesPerBuffer, paClipOff, audioCallback, this);

  if (err != paNoError) {
    std::cerr << "Failed to open stream: " << Pa_GetErrorText(err) << std::endl;

    // Fallback: try with even higher latency and larger buffer
    inputParameters.suggestedLatency = 0.2;  // 200ms
    outputParameters.suggestedLatency = 0.2; // 200ms
    unsigned long fallbackFramesPerBuffer = 1024;

    std::cout << "Trying fallback with higher latency (200ms) and larger "
                 "buffer (1024)"
              << std::endl;

    err =
        Pa_OpenStream(&stream, &inputParameters, &outputParameters, sampleRate,
                      fallbackFramesPerBuffer, paClipOff, audioCallback, this);

    if (err != paNoError) {
      // Last resort: try output-only
      std::cout << "Trying output-only stream as last resort" << std::endl;
      err = Pa_OpenStream(&stream, nullptr, &outputParameters, sampleRate,
                          fallbackFramesPerBuffer, paClipOff, audioCallback,
                          this);

      if (err != paNoError) {
        throw std::runtime_error("Failed to open stream with PortAudio: " +
                                 std::string(Pa_GetErrorText(err)));
      }
    }
  }

  err = Pa_StartStream(stream);
  if (err != paNoError) {
    std::cerr << "Failed to start stream: " << Pa_GetErrorText(err)
              << std::endl;
    Pa_CloseStream(stream);
    throw std::runtime_error("Failed to start PortAudio stream: " +
                             std::string(Pa_GetErrorText(err)));
  }

  const PaStreamInfo *streamInfo = Pa_GetStreamInfo(stream);
  if (streamInfo) {
    std::cout << "Stream successfully started with:"
              << "\n  - Input latency: " << streamInfo->inputLatency << "s"
              << "\n  - Output latency: " << streamInfo->outputLatency << "s"
              << "\n  - Sample rate: " << streamInfo->sampleRate << "Hz"
              << std::endl;
  } else {
    std::cout << "Stream started but couldn't get stream info" << std::endl;
  }
}

void AudioEngine::stop() {
  if (this->stream) {
    Pa_StopStream(this->stream);
    Pa_CloseStream(this->stream);
    this->stream = nullptr;
  }
}

Processor *AudioEngine::getProcessor() { return processor; }

int AudioEngine::audioCallback(const void *inputBuffer, void *outputBuffer,
                               unsigned long framesPerBuffer,
                               const PaStreamCallbackTimeInfo *timeInfo,
                               PaStreamCallbackFlags flags, void *userData) {
  AudioEngine *engine = static_cast<AudioEngine *>(userData);
  float **out = (float **)outputBuffer;
  float **in = (float **)inputBuffer;

  // Create a zero-filled buffer if input is null
  float *tempBuffers[2] = {nullptr, nullptr};
  float **actualInput = in;

  if (in == nullptr) {
    // Create temporary buffers for input if none provided
    tempBuffers[0] = new float[framesPerBuffer](); // Initialize to zero
    tempBuffers[1] = new float[framesPerBuffer]();
    actualInput = tempBuffers;
  }

  // Process audio
  engine->processor->process(actualInput, out, framesPerBuffer);

  // Clean up temporary buffers if created
  if (in == nullptr) {
    delete[] tempBuffers[0];
    delete[] tempBuffers[1];
  }

  return paContinue;
}
