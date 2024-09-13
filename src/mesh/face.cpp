#include <gpolylla/mesh/face.h>

#include <algorithm>
#include <ostream>
#include <unordered_map>
#include <vector>

#include "gpolylla/mesh/basic.h"

namespace gpolylla {
using std::ostream;
using std::unordered_map;
using std::vector;

FaceVertex::FaceVertex(const Vertex& v) : Vertex(v.idx, v.x, v.y, v.z) {}

FaceFace::FaceFace(const Face& f)
    : Face(f.idx, f.vertices[0], f.vertices[1], f.vertices[2]),
      tetras({-1, -1}) {}

ostream& operator<<(ostream& out, const FaceFace& f) {
  out << "(Face " << f.idx << ",";
  for (auto& vi : f.vertices) out << " " << vi;
  out << ", tetras:";
  for (auto& ti : f.tetras) out << " " << ti;
  out << ")";
  return out;
}

FaceTetrahedron::FaceTetrahedron(const Tetrahedron& t)
    : Tetrahedron(t.idx, t.vertices[0], t.vertices[1], t.vertices[2],
                  t.vertices[3]),
      fittest(-1),
      faces({-1, -1, -1, -1}) {}

ostream& operator<<(ostream& out, const FaceTetrahedron& t) {
  out << "(Tetrahedron " << t.idx << ",";
  for (auto& vi : t.vertices) out << " " << vi;
  out << ", faces:";
  for (auto& fi : t.faces) out << " " << fi;
  out << ")";
  return out;
}


FaceTetraMesh::FaceTetraMesh(const TetraMesh& m) {
  vertices.reserve(m.vertices.size());
  faces.reserve(m.faces.size());
  tetras.reserve(m.tetras.size());

  for (const auto& v : m.vertices) {
    vertices.push_back(v);
  }

  unordered_map<Face, vector<int>, Face::Hash> neighs;

  for (int ti = 0; ti < m.tetras.size(); ti++) {
    const Tetrahedron& t = m.tetras[ti];
    for (int vi = 0; vi < 4; vi++) {
      Face temp(0);
      temp.vertices[0] = t.vertices[vi];
      temp.vertices[1] = t.vertices[(vi + 1) % 4];
      temp.vertices[2] = t.vertices[(vi + 2) % 4];
      std::sort(temp.vertices.begin(), temp.vertices.end());
      neighs[temp].push_back(ti);
    }
  }

  unordered_map<Tetrahedron, vector<int>, Tetrahedron::Hash> faces_of_tetra(
      m.tetras.size());

  for (const auto& f : m.faces) {
    Face oldFace = f;
    FaceFace newFace(oldFace);
    std::sort(oldFace.vertices.begin(), oldFace.vertices.end());
    const auto& tetras = neighs[oldFace];

    int i = 0;
    for (int ti : tetras) {
      newFace.tetras[i] = ti;
      faces_of_tetra[m.tetras[ti]].push_back(newFace.idx);
      i++;
    }

    faces.push_back(newFace);
  }

  for (const auto& t : m.tetras) {
    FaceTetrahedron newTetra(t);

    int i = 0;
    for (int fi : faces_of_tetra[t]) {
      newTetra.faces[i] = fi;
      i++;
    }

    tetras.push_back(newTetra);
  }
};

ostream& operator<<(ostream& out, const FaceTetraMesh& m) {
  out << "FACEMESH\n"
      << "    Total vertices: " << m.vertices.size() << "\n"
      << "    Total edges: " << m.vertices.size() << "\n"
      << "    Total faces: " << m.vertices.size() << "\n"
      << "    Total tetrahedrons: " << m.vertices.size() << "\n";
  out << "VERTICES\n";
  for (const auto& v : m.vertices) {
    out << "    " << v << "\n";
  }
  // out << "EDGES\n";
  // for (const auto& e : m.edges) {
  //   out << "    " << e << "\n";
  // }
  out << "FACES\n";
  for (const auto& f : m.faces) {
    out << "    " << f << "\n";
  }
  out << "TETRAHEDRONS\n";
  for (const auto& t : m.tetras) {
    out << "    " << t << "\n";
  }
  return out;
}

}  // namespace gpolylla
