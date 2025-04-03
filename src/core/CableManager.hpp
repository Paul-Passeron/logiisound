#include <imgui.h>
#include <utility>
#include <vector>

using std::pair;
using std::vector;

class CableManager {
  vector<pair<ImVec2, ImVec2>> cables;
  void cleanUpCables();

public:
  void addCable(const ImVec2 &a, const ImVec2 &b);
  void deleteCable(int index);
  const vector<pair<ImVec2, ImVec2>> getCables() const;
  int findNearestCable(const ImVec2 &point, float *outDistance = nullptr) const;
  float calculateDistanceToSegment(const ImVec2 &p, const ImVec2 &a,
                                   const ImVec2 &b) const;
};
