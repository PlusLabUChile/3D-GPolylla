#ifndef UTILS_H
#define UTILS_H
#include <gpolylla/polylla.h>

#include <map>

namespace Polylla {

template <typename T>
bool sameContent(const T* a, const T* b, size_t size) {
  bool same = true;
  for (size_t i = 0; i < size; ++i) {
    bool equal = false;
    for (size_t j = 0; j < size; ++j) {
      if (a[i] == b[j]) {
        equal = true;
        break;
      }
    }

    if (!equal) {
      same = false;
      break;
    }
  }
  return same;
}

std::vector<std::array<int, 3>> makeDirectedFaces(
    const std::vector<Tetrahedron>& tetras,
    const std::vector<Vertex>& vertices);

std::map<std::array<int, 3>, std::array<int, 2>> calculateNeighbours(
    const std::vector<Tetrahedron>& tetras);

}  // namespace Polylla

#endif  // UTILS_H
