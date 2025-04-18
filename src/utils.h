#ifndef UTILS_H
#define UTILS_H
#include <array>
#include <map>

#include "gpolylla.h"

namespace gpolylla {
using FaceKey = std::array<int, 3>;
using FaceMap = std::map<FaceKey, std::array<int, 2>>;
FaceKey getKey(const Face& f);
FaceMap getAdjacentFaces(const std::vector<Face>& faces);

}  // namespace gpolylla

#endif  // MATH_H
