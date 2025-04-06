#pragma once

#include "Editor.hpp"
#include <fstream>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

using json = nlohmann::json;
using std::string;

class CircuitSerializer {
public:
  static bool saveCircuit(const Editor &editor, const string &filePath);
  static bool loadCircuit(Editor &editor, const string &filePath);

private:
  static json serializeComponent(const PlacedComponent &component);
  static json serializeCable(const std::pair<ImVec2, ImVec2> &cable);
  static json serializeVector2(const ImVec2 &vec);

  static PlacedComponent deserializeComponent(const json &data);
  static std::pair<ImVec2, ImVec2> deserializeCable(const json &data);
  static ImVec2 deserializeVector2(const json &data);
};
