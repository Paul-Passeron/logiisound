#include "Application.hpp"
#include "../audio/processors/AddProcessor.hpp"
#include "../audio/processors/ChainProcessor.hpp"
#include "../audio/processors/CircuitProcessor.hpp"
#include "../audio/processors/FilePlayer.hpp"
#include "../audio/processors/GainProcessor.hpp"
#include "../audio/processors/ScopeProcessor.hpp"
#include "../audio/processors/SineGenerator.hpp"
#include "../audio/processors/SquareGenerator.hpp"
#include "../audio/processors/SwitchProcessor.hpp"
#include "../audio/processors/customs/PedalProcessors.hpp"
#include "../circuits/Capacitor.hpp"
#include "../circuits/Circuit.hpp"
#include "../circuits/Diode.hpp"
#include "../circuits/Resistor.hpp"
#include "../circuits/VoltageSource.hpp"
#include "../circuits/transistors/BJTs/NPN.hpp"
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL_events.h>
#include <SDL_keyboard.h>
#include <SDL_video.h>
#include <eigen3/Eigen/src/Core/Matrix.h>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl2.h>
#include <iostream>
#include <sndfile.h>
#include <tinyfiledialogs.h>

const char *Application::getTitle() { return "LogIISound"; }

Application::Application() {
  this->init();
  instance = this;
}

void Application::init() {
  printf("[INFO]: Initializing application...\n");
  this->initSDL2();
  this->initImGui();
  printf("[INFO]: Application initialization successful !\n");
  ChainProcessor *processor = new ChainProcessor();
  // processor->addProcessor(new FilePlayer());
  processor->addProcessor(new ScopeProcessor());

  ChainProcessor *circuitChain = new ChainProcessor();
  circuitChain->addProcessor(new GainProcessor());
  circuitChain->addProcessor(PedalProcessors::FuzzProcessor());
  SwitchProcessor *sw = new SwitchProcessor(circuitChain);

  processor->addProcessor(sw);
  processor->addProcessor(new GainProcessor());
  processor->addProcessor(new ScopeProcessor());
  this->engine = new AudioEngine(processor);
}

void Application::initSDL2() {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
    fprintf(stderr, "Error: %s\n", SDL_GetError());
    exit(-1);
  }

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS,
                      SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

  this->window = SDL_CreateWindow(this->getTitle(), SDL_WINDOWPOS_CENTERED,
                                  SDL_WINDOWPOS_CENTERED, 1280, 720,
                                  SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
  this->glContext = SDL_GL_CreateContext(this->window);
  SDL_GL_MakeCurrent(window, this->glContext);
  SDL_GL_SetSwapInterval(1);

  if (glewInit() != GLEW_OK) {
    printf("Failed to initialize GLEW\n");
    exit(-1);
  }
}

void Application::initImGui() {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  this->io = &ImGui::GetIO();
  this->io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

  ImGui_ImplSDL2_InitForOpenGL(this->window, this->glContext);
  ImGui_ImplOpenGL3_Init(this->getGlslVersion());
}

const char *Application::getGlslVersion() { return "#version 150"; }

void Application::renderNewBlankFrame() {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplSDL2_NewFrame();
  ImGui::NewFrame();
  glViewport(0, 0, 1280, 720);
  glClearColor(0.28f, 0.28f, 0.28f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
}

void Application::renderFrame() {
  ImGui::SetNextWindowPos(ImVec2(0, 0));
  int w, h;
  SDL_GetWindowSize(window, &w, &h);
  ImGui::SetNextWindowSize(ImVec2(w, 0), false);
  ImGui::Begin("Main", nullptr,
               ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoResize |
                   ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar |
                   ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground);
  if (ImGui::BeginMenuBar()) {
    if (ImGui::BeginMenu("File")) {
      if (ImGui::MenuItem("Exit"))
        this->isRunning = false;
      ImGui::EndMenu();
    }
    ImGui::EndMenuBar();
  }
  ImGui::End();
  ImGui::Begin("Processors", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
  ImGui::Separator();
  engine->getProcessor()->render();
  if (ImGui::Button("Play")) {
    this->onPlayPressed();
  }
  ImGui::End();

  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  SDL_GL_SwapWindow(this->window);
}

void Application::run() {
  this->isRunning = true;
  SDL_Event event;
  while (this->isRunning) {
    while (SDL_PollEvent(&event)) {
      ImGui_ImplSDL2_ProcessEvent(&event);
      this->handleEvent(event);
    }
    this->renderNewBlankFrame();
    this->renderFrame();
  }
  this->shutdown();
}

void Application::shutdownImGui() {
  printf("[INFO]: Shutting down, ImGui...\n");
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();
  printf("[INFO]: ImGui shutdown successful !\n");
}

void Application::shutdownSDL2() {
  printf("[INFO]: Shutting down, SDL2...\n");
  SDL_GL_DeleteContext(this->glContext);
  SDL_DestroyWindow(this->window);
  SDL_Quit();
  printf("[INFO]: SDL2 shutdown successful !\n");
}

void Application::shutdown() {
  this->engine->stop();
  delete engine;
  printf("[INFO]: Shutting down application...\n");
  this->shutdownImGui();
  this->shutdownSDL2();
  printf("[INFO]: Application shutdown successful !\n");
}

void Application::handleEvent(SDL_Event event) {
  if (event.type == SDL_QUIT) {
    this->isRunning = false;
  }
  if (event.type == SDL_KEYDOWN) {
    if (event.key.keysym.sym == SDLK_q) {
      if (event.key.keysym.mod & KMOD_CTRL) {
        this->isRunning = false;
      }
    }
  }
}

bool Application::loadAudioFile(const std::string &path,
                                vector<float> &audioData) {
  SF_INFO sfInfo;
  SNDFILE *file = sf_open(path.c_str(), SFM_READ, &sfInfo);

  if (!file) {
    std::cerr << "Could not open audio file: " << path << std::endl;
    return false;
  }
  audioData.resize(sfInfo.frames * sfInfo.channels);
  sf_read_float(file, audioData.data(), audioData.size());
  sf_close(file);
  return true;
}

void Application::onPlayPressed() {
  if (!this->isAudioPlaying) {
    this->isAudioPlaying = true;
    engine->start();
  } else {
    this->isAudioPlaying = false;
    engine->stop();
  }
}

void Application::setHistory(vector<Eigen::VectorXd> history) {
  this->history = history;
}

Application *Application::instance = nullptr;

Application *Application::getInstance() { return instance; }

void Application::setNumState(int n) { numStates = n; }

SDL_Window *Application::getWindow() { return window; }
