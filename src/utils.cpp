#include "utils.h"

#include <set>

using namespace Polylla;
using namespace std;
using vec3 = Eigen::Vector3f;

using Face = array<int, 3>;

vec3 normal(const Vertex& v0, const Vertex& v1, const Vertex& v2) {
  return (v1 - v0).cross(v2 - v0).normalized();
}

float isOutside(const Vertex& v0, const Vertex& v1, const Vertex& v2,
                const Vertex& other) {
  vec3 n = normal(v0, v1, v2);
  return (other - v0).dot(n);
}

vector<Face> Polylla::makeDirectedFaces(const vector<Tetrahedron>& cells,
                                        const vector<Vertex>& vertices) {
  vector<Face> faces;
  int cases[][] = {
      {1, 2, 3},  // 0
      {0, 2, 3},  // 1
      {0, 1, 3},  // 2
      {0, 1, 2}   // 3
  };

  for (const auto& c : cells) {
    for (int vi = 0; vi < 4; ++vi) {
      const int* config = cases[vi];
      Vertex v0 = vertices[c.vertices[config[0]]];
      Vertex v1 = vertices[c.vertices[config[1]]];
      Vertex v2 = vertices[c.vertices[config[2]]];
      Vertex other = vertices[c.vertices[vi]];
      if (isOutside(v0, v1, v2, other)) {
        faces.emplace_back(v0, v1, v2)
      }
    }
  }

  return faces;
}
