#pragma once

#include "../audio/engine/AudioEngine.hpp"
#include <SDL_video.h>
#include <eigen3/Eigen/Dense>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <string>

using std::string;

class Application {
  SDL_Window *window = nullptr;
  SDL_GLContext glContext;
  ImGuiIO *io = nullptr;
  bool isRunning = false;

  AudioEngine *engine = nullptr;

  bool isAudioPlaying = false;
  vector<Eigen::VectorXd> history;
  static Application *instance;
  int numStates;

public:
  Application();
  void shutdown();
  void run();
  void setHistory(vector<Eigen::VectorXd> history);
  void setNumState(int n);
  static Application *getInstance();
  SDL_Window *getWindow();

private:
  void init();
  void initSDL2();
  void shutdownSDL2();
  void initImGui();
  void shutdownImGui();
  static const char *getGlslVersion();
  void handleEvent(SDL_Event event);
  void renderNewBlankFrame();
  void renderFrame();
  static const char *getTitle();
  static bool loadAudioFile(const std::string &path, vector<float> &audioData);
  void onPlayPressed();
  void onBrowsePressed();
};
