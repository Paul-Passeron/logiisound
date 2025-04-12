##################################################
cmake_minimum_required(VERSION 3.10)
project(logiisound)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")


# Generate compile_commands.json for clang
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_BUILD_TYPE Debug)
##################################################

##################################################
# LIBS

# Find SDL2
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

# Find SDL2_image
find_package(SDL2_image REQUIRED)
include_directories(${SDL2_image_INCLUDE_DIRS})

# Find OpenGL
find_package(OpenGL REQUIRED)

# Find GLEW
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLEW REQUIRED glew)
include_directories(${GLEW_INCLUDE_DIRS})

# Find PortAudio
pkg_check_modules(PORTAUDIO REQUIRED portaudio-2.0)
include_directories(${PORTAUDIO_INCLUDE_DIRS})

# Add ImGui
add_subdirectory(vendor/imgui)
include_directories(vendor/imgui vendor/imgui/backends)

add_subdirectory(vendor/nlohmann/json)
include_directories(vendor/nlohmann/json)

# Add tinyfiledialogs
add_subdirectory(vendor/libtinyfiledialogs)
include_directories(vendor/libtinyfiledialogs)

# Add sndfile
pkg_check_modules(SNDFILE REQUIRED sndfile)
include_directories(${SNDFILE_INCLUDE_DIRS})

##################################################

##################################################
# Source files
set(SOURCES
  src/main.cpp
  src/core/Application.cpp src/core/Application.hpp
  src/core/Editor.cpp src/core/Editor.hpp
  src/core/CableHelper.cpp src/core/CableHelper.hpp
  src/core/CableManager.cpp src/core/CableManager.hpp
  src/core/CircuitSerializer.cpp src/core/CircuitSerializer.hpp

  src/audio/processors/Processor.hpp src/audio/processors/Processor.cpp
  src/audio/processors/GainProcessor.cpp src/audio/processors/GainProcessor.hpp
  src/audio/processors/SwitchProcessor.cpp src/audio/processors/SwitchProcessor.hpp
  src/audio/processors/AddProcessor.cpp src/audio/processors/AddProcessor.hpp
  src/audio/processors/ChainProcessor.cpp src/audio/processors/ChainProcessor.hpp
  src/audio/processors/ScopeProcessor.cpp src/audio/processors/ScopeProcessor.hpp
  src/audio/processors/CircuitProcessor.cpp src/audio/processors/CircuitProcessor.hpp
  src/audio/processors/FilePlayer.cpp src/audio/processors/FilePlayer.hpp
  src/audio/processors/SineGenerator.cpp src/audio/processors/SineGenerator.hpp
  src/audio/processors/SquareGenerator.cpp src/audio/processors/SquareGenerator.hpp
  src/audio/processors/customs/PedalProcessors.cpp src/audio/processors/customs/PedalProcessors.hpp
  src/audio/engine/AudioEngine.cpp src/audio/engine/AudioEngine.hpp

  src/circuits/Circuit.cpp src/circuits/Circuit.hpp

  src/circuits/models/ComponentModel.cpp src/circuits/models/ComponentModel.hpp
  src/circuits/models/DiodeModel.cpp src/circuits/models/DiodeModel.hpp
  src/circuits/models/ResistorModel.cpp src/circuits/models/ResistorModel.hpp
  src/circuits/models/CapacitorModel.cpp src/circuits/models/CapacitorModel.hpp
  src/circuits/models/VoltageSourceModel.cpp src/circuits/models/VoltageSourceModel.hpp
  src/circuits/models/transistors/BJTs/NPNModel.cpp src/circuits/models/transistors/BJTs/NPNModel.hpp

  src/circuits/ComponentRegistry.cpp src/circuits/ComponentRegistry.hpp

  src/circuits/factories/ComponentFactory.hpp

  src/circuits/factories/ComponentFactory.hpp
  src/circuits/factories/CapacitorFactory.hpp
  src/circuits/factories/CapacitorFactory.cpp
  src/circuits/factories/ResistorFactory.cpp
  src/circuits/factories/ResistorFactory.hpp
  src/circuits/factories/DiodeFactory.hpp
  src/circuits/factories/DiodeFactory.cpp
  src/circuits/factories/VoltageSourceFactory.hpp
  src/circuits/factories/VoltageSourceFactory.cpp
  src/circuits/factories/NPNFactory.hpp
  src/circuits/factories/NPNFactory.cpp
)

add_executable(logiisound ${SOURCES})
##################################################

##################################################
# Link libraries
target_link_libraries(logiisound
        ${SDL2_image_LIBRARIES}
        ${GLEW_LIBRARIES}
        ${OPENGL_LIBRARIES}
        ${SDL2_LIBRARIES}
        ${PORTAUDIO_LIBRARIES}
        ${SNDFILE_LIBRARIES}
        imgui
        glfw
        dl
        libtinyfiledialogs
        SDL2_image
        nlohmann_json
)

##################################################
