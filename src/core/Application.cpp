#include "Application.hpp"
#include "../audio/processors/ChainProcessor.hpp"
#include "../audio/processors/FilePlayer.hpp"
#include "../audio/processors/GainProcessor.hpp"
#include "../audio/processors/ScopeProcessor.hpp"
#include "../audio/processors/SwitchProcessor.hpp"
#include "../audio/processors/customs/PedalProcessors.hpp"
#include "../circuits/ComponentRegistry.hpp"
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL_error.h>
#include <SDL_events.h>
#include <SDL_keyboard.h>
#include <SDL_render.h>
#include <SDL_video.h>
#include <eigen3/Eigen/src/Core/Matrix.h>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>
#include <iostream>
#include <portaudio.h>
#include <sndfile.h>
#include <thread>
#include <tinyfiledialogs.h>

const char *Application::getTitle() { return "LogIISound"; }

Application::Application() {
  instance = this;
  this->init();
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
  circuitBox = new ChainProcessor();
  circuitBox->addProcessor(PedalProcessors::FuzzProcessor());
  circuitChain->addProcessor(circuitBox);
  SwitchProcessor *sw = new SwitchProcessor(circuitChain);

  processor->addProcessor(sw);
  processor->addProcessor(new GainProcessor());
  processor->addProcessor(new ScopeProcessor());
  this->engine = new AudioEngine(processor);

  registerComponents();
  editor = Editor();
}

void Application::initSDL2() {
  // Initialize SDL2 with video and timer support
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
    fprintf(stderr, "Error initializing SDL: %s\n", SDL_GetError());
    exit(-1);
  }

  // Create an SDL window
  this->window =
      SDL_CreateWindow(this->getTitle(), SDL_WINDOWPOS_CENTERED,
                       SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_RESIZABLE);

  if (!this->window) {
    fprintf(stderr, "Error creating SDL Window: %s\n", SDL_GetError());
    SDL_Quit();
    exit(-1);
  }
  // Create an SDL renderer
  this->renderer = SDL_CreateRenderer(
      this->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

  if (!this->renderer) {
    fprintf(stderr, "Error creating SDL Renderer: %s\n", SDL_GetError());
    SDL_DestroyWindow(this->window);
    SDL_Quit();
    exit(-1);
  }

  printf("[INFO]: SDL2 initialization successful!\n");
}

void Application::initImGui() {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  this->io = &ImGui::GetIO();
  this->io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  this->io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  ImGui::StyleColorsLight();

  // Initialize ImGui SDL2 and SDL_Renderer bindings
  ImGui_ImplSDL2_InitForSDLRenderer(this->window, this->renderer);
  ImGui_ImplSDLRenderer2_Init(this->renderer);

  printf("[INFO]: ImGui initialization successful!\n");
}

const char *Application::getGlslVersion() { return "#version 150"; }

void Application::renderNewBlankFrame() {
  ImGui_ImplSDL2_NewFrame();
  ImGui_ImplSDLRenderer2_NewFrame();
  ImGui::NewFrame();
  SDL_SetRenderDrawColor(renderer, 120, 120, 120, 255);
  SDL_RenderClear(renderer);
}

void Application::renderComponentView() {
  ImGui::Begin("Component Palette", nullptr);
  int i = 0;
  ImGui::Text("Components:");
  ImGui::BeginGroup();
  for (auto comp : ComponentRegistry::instance().getRegistry()) {
    ImGui::PushID(i++);
    pair<int, int> size = comp.second->getSize();
    int xSize = size.first;
    int ySize = size.second;
    int w = 16.0 * xSize;
    int h = 16.0 * ySize;
    if (ImGui::ImageButton("##", (ImTextureID)comp.second->getTexture(),
                           ImVec2(w, h))) {
      editor.setComponentId(comp.first);
    }
    ImGui::PopID();
    if (i % 4 != 0) {
      ImGui::SameLine();
    }
  }
  ImGui::EndGroup();

  ImGui::End();
}

void Application::renderFrame() {

  ImGuiViewport *viewport = ImGui::GetMainViewport();
  ImGui::SetNextWindowPos(viewport->Pos);
  ImGui::SetNextWindowSize(viewport->Size);
  ImGui::SetNextWindowViewport(viewport->ID);

  ImGuiWindowFlags window_flags =
      ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
      ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
      ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
      ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_MenuBar;

  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  ImGui::Begin("MainDockSpace", nullptr, window_flags);
  ImGui::PopStyleVar(3);
  ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");
  ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f),
                   ImGuiDockNodeFlags_PassthruCentralNode);
  if (ImGui::BeginMenuBar()) {
    if (ImGui::BeginMenu("File")) {
      if (ImGui::MenuItem("Open")) {
        // editor.
        const char *filePath = tinyfd_openFileDialog(
            "Select a circuit", "../assets/", 0, nullptr, nullptr, 0);
        if (filePath) {
          if (!editor.loadCircuit(filePath)) {
            tinyfd_messageBox("Error", "Failed to load circuit!", "ok", "error",
                              1);
          }
        }
      }
      if (ImGui::MenuItem("Save")) {
        // TODO: actually solve "Not Responding issue"
        std::string result = "";
        std::thread dialogThread([&result]() {
          const char *filePath = tinyfd_saveFileDialog(
              "Save circuit", "../assets/", 0, nullptr, "");
          if (filePath) {
            result = filePath;
          }
        });

        if (dialogThread.joinable()) {
          dialogThread.join();
        }
        if (!result.empty()) {
          if (!editor.saveCircuit(result)) {
            tinyfd_messageBox("Error", "Failed to save circuit!", "ok", "error",
                              1);
          }
        }
      }
      if (ImGui::MenuItem("Exit"))
        this->isRunning = false;
      ImGui::EndMenu();
    }
    ImGui::EndMenuBar();
  }
  ImGui::End();

  ImGui::Begin("Processors", nullptr);
  ImGui::Separator();
  engine->getProcessor()->render();
  if (ImGui::Button("Play")) {
    this->onPlayPressed();
  }
  ImGui::End();
  editor.render();
  ImGui::SetNextWindowDockID(dockspace_id, ImGuiCond_FirstUseEver);
  renderComponentView();
  ImGui::Render();
  ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
  SDL_RenderPresent(renderer);
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
  ImGui_ImplSDL2_Shutdown();
  ImGui_ImplSDLRenderer2_Shutdown();
  ImGui::DestroyContext();
  printf("[INFO]: ImGui shutdown successful !\n");
}

void Application::shutdownSDL2() {
  printf("[INFO]: Shutting down, SDL2...\n");
  // SDL_GL_DeleteContext(this->glContext);
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
  editor.handleEvent(event);
}

void Application::onPlayPressed() {
  circuitBox->clear();
  circuitBox->addProcessor(editor.toCircuit());
  if (!this->isAudioPlaying) {
    this->isAudioPlaying = true;
    engine->start();
  } else {
    this->isAudioPlaying = false;
    engine->stop();
  }
}

Application *Application::instance = nullptr;

Application *Application::getInstance() { return instance; }

SDL_Window *Application::getWindow() { return window; }

SDL_Renderer *Application::getRenderer() { return renderer; }
