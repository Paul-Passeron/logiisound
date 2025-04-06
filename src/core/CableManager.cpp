#include "CableManager.hpp"
#include "CableHelper.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>

using std::map;
using std::pair;
using std::vector;

void CableManager::addCable(const ImVec2 &a, const ImVec2 &b) {
  cables.emplace_back(a, b);
  cleanUpCables();
  updateJunctionNodes();
}

void CableManager::deleteCable(int index) {
  if (index >= 0 && index < cables.size()) {
    cables.erase(cables.begin() + index);
    cleanUpCables();
    updateJunctionNodes();
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
  if (outDistance) {
    *outDistance = minDistance;
  }
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

std::vector<std::pair<ImVec2, ImVec2>>
getUniques(const std::vector<std::pair<ImVec2, ImVec2>> &cables) {
  std::vector<std::pair<ImVec2, ImVec2>> uniqueCables;
  for (const auto &cable : cables) {
    bool found = false;
    for (const auto &existing : uniqueCables) {
      if ((cable.first.x == existing.first.x &&
           cable.first.y == existing.first.y &&
           cable.second.x == existing.second.x &&
           cable.second.y == existing.second.y) ||
          (cable.first.x == existing.second.x &&
           cable.first.y == existing.second.y &&
           cable.second.x == existing.first.x &&
           cable.second.y == existing.first.y)) {
        found = true;
        break;
      }
    }
    if (!found)
      uniqueCables.push_back(cable);
  }
  return uniqueCables;
}

void CableManager::cleanUpCables() {
  cables = getUniques(cables);
  vector<ImVec2> allEndpoints;
  for (const auto &cable : cables) {
    allEndpoints.push_back(cable.first);
    allEndpoints.push_back(cable.second);
  }
  for(const auto &n:  externalNodes) {
    allEndpoints.push_back(n);
  }
  std::vector<std::pair<ImVec2, ImVec2>> newCables;
  for (const auto &cable : cables) {
    auto [p1, p2] = cable;
    if ((p1.x > p2.x) || (p1.x == p2.x && p1.y > p2.y))
      std::swap(p1, p2);
    bool isV = (p1.x == p2.x);
    std::vector<float> splitPoints = {isV ? p1.y : p1.x, isV ? p2.y : p2.x};
    for (const auto &endpoint : allEndpoints) {
      if ((endpoint.x == p1.x && endpoint.y == p1.y) ||
          (endpoint.x == p2.x && endpoint.y == p2.y))
        continue;
      if (isV) {
        if (endpoint.x == p1.x && endpoint.y > p1.y && endpoint.y < p2.y) {
          splitPoints.push_back(endpoint.y);
        }
      } else {
        if (endpoint.y == p1.y && endpoint.x > p1.x && endpoint.x < p2.x) {
          splitPoints.push_back(endpoint.x);
        }
      }
    }
    std::sort(splitPoints.begin(), splitPoints.end());
    splitPoints.erase(std::unique(splitPoints.begin(), splitPoints.end()),
                      splitPoints.end());
    for (size_t i = 0; i < splitPoints.size() - 1; ++i) {
      if (isV) {
        newCables.push_back(
            {{p1.x, splitPoints[i]}, {p1.x, splitPoints[i + 1]}});
      } else {
        newCables.push_back(
            {{splitPoints[i], p1.y}, {splitPoints[i + 1], p1.y}});
      }
    }
  }
  cables = getUniques(newCables);
}

const vector<ImVec2> &CableManager::getJunctionNodes() const {
  return junctionNodes;
}

void CableManager::updateJunctionNodes() {
  junctionNodes.clear();
  map<ImVec2, int, PointCompare> nodeCount;
  for (const auto [a, b] : cables) {
    nodeCount[a]++;
    nodeCount[b]++;
  }
  for (const auto &a : externalNodes) {
    nodeCount[a]++;
  }
  for (const auto &[node, count] : nodeCount) {
    if (count > 2) {
      junctionNodes.push_back(node);
    }
  }
}

void CableManager::updateExternalNodes(const vector<ImVec2> &ext) {
  std::cout << "Update nodes" << std::endl;
  externalNodes.clear();
  externalNodes = ext;
  updateJunctionNodes();
  cleanUpCables();
  updateJunctionNodes();
  cleanUpCables();
}

void CableManager::clear() {
cables.clear();
junctionNodes.clear();
externalNodes.clear();
}
