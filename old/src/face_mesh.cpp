#include <gpolylla/mesh.h>

#include <algorithm>
#include <map>
#include <ostream>
#include <vector>

namespace gpolylla {
using std::array;
using std::map;
using std::ostream;
using std::tuple;
using std::vector;

using faceMap = map<array<int, 3>, array<int, 2>>;

void calculateAdjacentFaces(faceMap* twinMap, const vector<Face> &faces) {
  for (int fi = 0; fi < faces.size(); fi++) {
    const auto &f = faces[fi];
    auto sorted = f.vertices;
    std::sort(sorted.begin(), sorted.end());
    twinMap[sorted].push_back(fi);
  }
}

FaceFace::FaceFace(const Face &f)
    : Face(f.vertices[0], f.vertices[1], f.vertices[2]), twin(-1) {}

ostream &operator<<(ostream &out, const FaceFace &f) {
  out << "(Face ";
  for (auto &vi : f.vertices) out << " " << vi;
  out << ")";
  return out;
}

FaceTetraMesh::FaceTetraMesh(const TetraMesh &m)
    : vertices(m.vertices), tetras(m.tetras) {
  faces.reserve(m.faces.size());

  map<array<int, 3>, vector<int>> twinMap;

  for (int fi = 0; fi < m.faces.size(); fi++) {
    const auto &f = m.faces[fi];
    auto sorted = f.vertices;
    std::sort(sorted.begin(), sorted.end());
    twinMap[sorted].push_back(fi);
  }

  for (int fi = 0; fi < m.faces.size(); fi++) {
    const auto &f = m.faces[fi];
    auto sorted = f.vertices;
    std::sort(sorted.begin(), sorted.end());
    auto &twinFaces = twinMap[sorted];
    if (twinFaces.size() > 2) {
      std::cerr << "FACE: Face " << fi << " has " << twinFaces.size()
                << " twins\n";
      exit(0);
    }
    FaceFace newFace(f);
    if (twinFaces.size() == 2)
      newFace.twin = twinFaces[0] == fi ? twinFaces[1] : twinFaces[0];
    faces.push_back(newFace);
  }

  //        for (int ti = 0; ti < m.tetras.size(); ti++) {
  //            const Tetrahedron &t = m.tetras[ti];
  //            for (int vi = 0; vi < 4; vi++) {
  //                Face temp;
  //                temp.vertices[0] = t.vertices[vi];
  //                temp.vertices[1] = t.vertices[(vi + 1) % 4];
  //                temp.vertices[2] = t.vertices[(vi + 2) % 4];
  //                std::sort(temp.vertices.begin(), temp.vertices.end());
  //                neighs[temp].push_back(ti);
  //            }
  //        }
  //
  //        unordered_map <Tetrahedron, vector<int>, Tetrahedron::Hash>
  //        faces_of_tetra(
  //                m.tetras.size());
  //
  //        for (int fi = 0; fi < m.faces.size(); fi++) {
  //            Face oldFace = m.faces[fi];
  //            FaceFace newFace(oldFace);
  //            std::sort(oldFace.vertices.begin(), oldFace.vertices.end());
  //            const auto &tetras = neighs[oldFace];
  //
  //            int i = 0;
  //            for (int ti: tetras) {
  //                newFace.tetras[i] = ti;
  //                faces_of_tetra[m.tetras[ti]].push_back(fi);
  //                i++;
  //            }
  //
  //            faces.push_back(newFace);
  //        }

  //        for (const auto &t: m.tetras) {
  //            FaceTetrahedron newTetra(t);
  //
  //            int i = 0;
  //            for (int fi: faces_of_tetra[t]) {
  //                newTetra.faces[i] = fi;
  //                i++;
  //            }
  //
  //            tetras.push_back(newTetra);
  //        }
};

ostream &operator<<(ostream &out, const CavityTetraMesh &m) {
  out << "FACEMESH\n"
      << "    Total vertices: " << m.vertices.size() << "\n"
      << "    Total edges: " << m.vertices.size() << "\n"
      << "    Total faces: " << m.vertices.size() << "\n"
      << "    Total tetrahedrons: " << m.vertices.size() << "\n";
  out << "VERTICES\n";
  for (const auto &v : m.vertices) {
    out << "    " << v << "\n";
  }
  // out << "EDGES\n";
  // for (const auto& e : m.edges) {
  //   out << "    " << e << "\n";
  // }
  out << "FACES\n";
  for (const auto &f : m.faces) {
    out << "    " << f << "\n";
  }
  out << "TETRAHEDRONS\n";
  for (const auto &t : m.tetras) {
    out << "    " << t << "\n";
  }
  return out;
}

}  // namespace gpolylla
