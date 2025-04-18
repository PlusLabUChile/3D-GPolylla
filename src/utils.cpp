#include "utils.h"

#include <algorithm>

namespace gpolylla {
using namespace std;

FaceKey getKey(const Face &f) {
  array<int, 3> sorted = f.vertices;
  ranges::sort(sorted);
  return sorted;
}

FaceMap getAdjacentFaces(const vector<Face> &faces) {
  FaceMap twinMap;
  for (int fi = 0; fi < faces.size(); fi++) {
    const auto &f = faces.at(fi);
    auto key = getKey(f);
    int idx = 1;
    if (!twinMap.contains(key)) {
      twinMap[key] = {-1, -1};
      idx = 0;
    }

    twinMap[key][idx] = fi;
  }
  return twinMap;
}

}  // namespace gpolylla
