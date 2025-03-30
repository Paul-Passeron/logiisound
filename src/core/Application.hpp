#pragma once

#include "../audio/engine/AudioEngine.hpp"
#include "../circuits/ComponentModel.hpp"
#include "Editor.hpp"
#include <SDL_video.h>
#include <eigen3/Eigen/Dense>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <string>

using std::string;
using std::pair;

class Application {
  SDL_Window *window = nullptr;
  SDL_GLContext glContext;
  ImGuiIO *io = nullptr;
  bool isRunning = false;
  SDL_Renderer *renderer;
  AudioEngine *engine = nullptr;
  Editor editor;

  bool isAudioPlaying = false;
  vector<Eigen::VectorXd> history;
  static Application *instance;
  int numStates;

  vector<pair<string, ComponentModel *>> componentDisplayed;

public:
  Application();
  void shutdown();
  void run();
  void setHistory(vector<Eigen::VectorXd> history);
  void setNumState(int n);
  static Application *getInstance();
  SDL_Window *getWindow();
  SDL_Renderer *getRenderer();

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
  void onPlayPressed();
  void onBrowsePressed();
  void renderComponentView();
};
