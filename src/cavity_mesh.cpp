#include <gpolylla/mesh.h>

#include <unordered_map>

namespace gpolylla {
using std::unordered_map;
using std::vector;
using std::ostream;

CavityVertex::CavityVertex(const Vertex& v) : Vertex(v) {}

CavityFace::CavityFace(const Face& f) : Face(f) {}

CavityTetrahedron::CavityTetrahedron(const Tetrahedron& t)
    : Tetrahedron(t), neighs({-1, -1, -1, -1}) {}

CavityTetraMesh::CavityTetraMesh(const TetraMesh& m) {
  vertices.reserve(m.vertices.size());
  faces.reserve(m.faces.size());
  tetras.reserve(m.tetras.size());

  FaceTetraMesh aux(m);

  for (const auto& v : m.vertices) {
    vertices.push_back(v);
  }

  for (const auto& f : m.faces) {
    faces.push_back(f);
  }

  for (const auto& t : aux.tetras) {
    CavityTetrahedron newTetra = m.tetras[t.idx];
    int i = 0;
    for (int fi : t.faces) {
      for (int ti : aux.faces[fi].tetras) {
        if (ti == newTetra.idx) continue;
        if (ti == -1) continue;
        newTetra.neighs[i] = ti;
        i++;
      }
    }

    tetras.push_back(newTetra);
  }
}

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
