#include "CircuitSerializer.hpp"
#include <fstream>
#include <iostream>

bool CircuitSerializer::saveCircuit(const Editor &editor,
                                    const string &filePath) {
  try {
    json circuitData;

    // Serialize components
    json componentsArray = json::array();
    for (const auto &component : editor.getPlacedComponents()) {
      componentsArray.push_back(serializeComponent(component));
    }
    circuitData["components"] = componentsArray;

    // Serialize cables
    json cablesArray = json::array();
    for (const auto &cable : editor.getCables()) {
      cablesArray.push_back(serializeCable(cable));
    }
    circuitData["cables"] = cablesArray;

    // Write to file
    std::ofstream file(filePath);
    if (!file.is_open()) {
      std::cerr << "Failed to open file for writing: " << filePath << std::endl;
      return false;
    }

    file << circuitData.dump(4); // Pretty-print with 4-space indentation
    file.close();

    std::cout << "Circuit saved to " << filePath << std::endl;
    return true;
  } catch (const std::exception &e) {
    std::cerr << "Error saving circuit: " << e.what() << std::endl;
    return false;
  }
}

bool CircuitSerializer::loadCircuit(Editor &editor, const string &filePath) {
  try {
    // Read file
    std::ifstream file(filePath);
    if (!file.is_open()) {
      std::cerr << "Failed to open file for reading: " << filePath << std::endl;
      return false;
    }

    json circuitData = json::parse(file);
    file.close();

    // Clear current circuit
    editor.clearCircuit();

    // Deserialize components
    if (circuitData.contains("components") &&
        circuitData["components"].is_array()) {
      for (const auto &compData : circuitData["components"]) {
        editor.addComponent(deserializeComponent(compData));
      }
    }

    // Deserialize cables
    if (circuitData.contains("cables") && circuitData["cables"].is_array()) {
      for (const auto &cableData : circuitData["cables"]) {
        editor.addCable(deserializeCable(cableData));
      }
    }

    std::cout << "Circuit loaded from " << filePath << std::endl;
    return true;
  } catch (const std::exception &e) {
    std::cerr << "Error loading circuit: " << e.what() << std::endl;
    return false;
  }
}

json CircuitSerializer::serializeComponent(const PlacedComponent &component) {
  json comp;
  comp["type"] = component.type;
  comp["position"] = serializeVector2(component.position);
  comp["angle"] = component.angle;
  comp["id"] = component.id;
  comp["data"] = component.data;
  return comp;
}

json CircuitSerializer::serializeCable(const std::pair<ImVec2, ImVec2> &cable) {
  json cableJson;
  cableJson["start"] = serializeVector2(cable.first);
  cableJson["end"] = serializeVector2(cable.second);
  return cableJson;
}

json CircuitSerializer::serializeVector2(const ImVec2 &vec) {
  json vecJson;
  vecJson["x"] = vec.x;
  vecJson["y"] = vec.y;
  return vecJson;
}

PlacedComponent CircuitSerializer::deserializeComponent(const json &data) {
  PlacedComponent component;
  component.type = data["type"].get<string>();
  component.position = deserializeVector2(data["position"]);
  component.angle = data["angle"].get<float>();
  component.id = data["id"].get<int>();
  component.data = data["data"];
  return component;
}

std::pair<ImVec2, ImVec2>
CircuitSerializer::deserializeCable(const json &data) {
  ImVec2 start = deserializeVector2(data["start"]);
  ImVec2 end = deserializeVector2(data["end"]);
  return std::make_pair(start, end);
}

ImVec2 CircuitSerializer::deserializeVector2(const json &data) {
  float x = data["x"].get<float>();
  float y = data["y"].get<float>();
  return ImVec2(x, y);
}
