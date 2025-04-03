#include "CableManager.hpp"
#include <cmath>

void CableManager::addCable(const ImVec2 &a, const ImVec2 &b) {
  cables.emplace_back(a, b);
}

void CableManager::deleteCable(int index) {
  if (index >= 0 && index < cables.size()) {
    cables.erase(cables.begin() + index);
  }
}

const vector<pair<ImVec2, ImVec2>> CableManager::getCables() const {
  return cables;
}

int CableManager::findNearestCable(const ImVec2 &point,
                                   float *outDistance) const {
  int nearestIndex = -1;
  float minDistance = FLT_MAX;

  for (int i = 0; i < cables.size(); i++) {
    const auto &[a, b] = cables[i];
    float distance = calculateDistanceToSegment(point, a, b);

    if (distance < minDistance) {
      minDistance = distance;
      nearestIndex = i;
    }
  }

  if (outDistance)
    *outDistance = minDistance;

  return nearestIndex;
}

float CableManager::calculateDistanceToSegment(const ImVec2 &p, const ImVec2 &a,
                                               const ImVec2 &b) const {
  ImVec2 ab = ImVec2(b.x - a.x, b.y - a.y);
  ImVec2 ap = ImVec2(p.x - a.x, p.y - a.y);

  float ab_squared = ab.x * ab.x + ab.y * ab.y;
  float ap_dot_ab = ap.x * ab.x + ap.y * ab.y;

  float t = std::max(0.0f, std::min(1.0f, ap_dot_ab / ab_squared));

  ImVec2 closest = ImVec2(a.x + t * ab.x, a.y + t * ab.y);
  float dx = p.x - closest.x;
  float dy = p.y - closest.y;

  return std::sqrt(dx * dx + dy * dy);
}
