#pragma once

#include <imgui.h>
#include <utility>

using std::pair;

struct PointCompare {
  bool operator()(const ImVec2 &a, const ImVec2 &b) const;
};

struct CableCompare {
  bool operator()(const pair<ImVec2, ImVec2> &c1,
                  const pair<ImVec2, ImVec2> &c2) const;
};
