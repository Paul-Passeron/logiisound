#include "CableHelper.hpp"

bool PointCompare::operator()(const ImVec2 &a, const ImVec2 &b) const {
  return a.x == b.x ? a.y < b.y : a.x < b.x;
}

bool CableCompare::operator()(const std::pair<ImVec2, ImVec2> &c1,
                              const std::pair<ImVec2, ImVec2> &c2) const {
  const auto &[a1, b1] = c1;
  const auto &[a2, b2] = c2;

  if (a1.x != a2.x)
    return a1.x < a2.x;
  if (a1.y != a2.y)
    return a1.y < a2.y;
  if (b1.x != b2.x)
    return b1.x < b2.x;
  return b1.y < b2.y;
}
