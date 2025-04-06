#pragma once

#include <imgui.h>
#include <utility>
#include <vector>

using std::pair;
using std::vector;

class CableManager {
  vector<pair<ImVec2, ImVec2>> cables;
  vector<ImVec2> junctionNodes;
  vector<ImVec2> externalNodes;
  void cleanUpCables();
  void updateJunctionNodes();

public:
  void addCable(const ImVec2 &a, const ImVec2 &b);
  void deleteCable(int index);
  const vector<pair<ImVec2, ImVec2>> getCables() const;
  int findNearestCable(const ImVec2 &point, float *outDistance = nullptr) const;
  float calculateDistanceToSegment(const ImVec2 &p, const ImVec2 &a,
                                   const ImVec2 &b) const;
  const vector<ImVec2> &getJunctionNodes() const;
  void updateExternalNodes(const vector<ImVec2> &ext);
  void clear();
};
