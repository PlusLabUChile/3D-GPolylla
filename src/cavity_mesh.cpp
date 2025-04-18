#include <algorithm>

#include "gpolylla.h"
#include "utils.h"

namespace gpolylla {
using namespace std;

CavityMesh::CavityMesh(const Mesh& m) : faces(m.faces), vertices(m.vertices) {
  tetras.reserve(m.tetras.size());
  const FaceMap twinMap = getAdjacentFaces(faces);

  for (int ti = 0; ti < m.tetras.size(); ++ti) {
    const auto& [vertices, faces] = m.tetras.at(ti);
    CavityTetrahedron newTetra(vertices, faces);

    for (int i = 0; i < faces.size(); ++i) {
      const auto& f = m.faceOfTetra(ti, i);
      for (auto& twinFaces = twinMap.at(getKey(f));
           const int twinFace : twinFaces) {
        if (twinFace == f.tetra) continue;
        newTetra.neighbours[i] = m.faces.at(twinFace).tetra;
        break;
      }
    }

    tetras.push_back(newTetra);
  }
}

};  // namespace gpolylla
