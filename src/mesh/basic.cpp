#include <gpolylla/mesh/basic.h>
#include "Eigen/src/Core/Matrix.h"

namespace gpolylla {
using std::ostream;
using std::vector;
using Eigen::Vector3d;

Vertex::Vertex(int idx) : idx(idx) {}

Vertex::Vertex(int idx, float x, float y, float z)
    : idx(idx), x(x), y(y), z(z) {}

Vertex::Vertex(const Vertex& v) : idx(v.idx), x(v.x), y(v.y), z(v.z) {}

Vector3d Vertex::asVector() const {
  return Vector3d(x, y, z);
}

bool Vertex::operator==(const Vertex& o) const {
  return idx == o.idx && x == o.x && y == o.y && z == o.z;
}

bool Vertex::operator<(const Vertex& o) const { return idx < o.idx; }

std::size_t Vertex::Hash::operator()(const Vertex& v) const {
  return std::hash<int>()(v.idx);
}

ostream& operator<<(ostream& out, const Vertex& v) {
  out << "(Vertex " << v.idx << ", " << v.x << " " << v.y << " " << v.z << ")";
  return out;
}

Edge::Edge(int idx) : idx(idx) {}
Edge::Edge(int idx, int v0, int v1) : idx(idx), vertices({v0, v1}) {}
Edge::Edge(const Edge& e) : idx(e.idx), vertices(e.vertices) {}

bool Edge::operator==(const Edge& e) const {
  return vertices[0] == e.vertices[0] && vertices[1] == e.vertices[1];
}

std::size_t Edge::Hash::operator()(const Edge& e) const {
  return std::hash<int>()(e.idx);
}

ostream& operator<<(ostream& out, const Edge& e) {
  out << "(Edge " << e.idx << ",";
  for (auto& vi : e.vertices) out << " " << vi;
  out << ")";
  return out;
}

Face::Face(int idx) : idx(idx) {}
Face::Face(int idx, int v0, int v1, int v2)
    : idx(idx), vertices({v0, v1, v2}) {}

// Face::Face(const Face& f) : idx(f.idx), vertices(f.vertices) {}

bool Face::operator==(const Face& f) const {
  return vertices[0] == f.vertices[0] && vertices[1] == f.vertices[1] &&
         vertices[2] == f.vertices[2];
}

std::size_t Face::Hash::operator()(const Face& f) const {
  std::size_t seed = 0;
  std::hash<int> hasher;
  for (int vertex : f.vertices) {
    seed ^= hasher(vertex) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
  }
  return seed;
}

ostream& operator<<(ostream& out, const Face& f) {
  out << "(Face " << f.idx << ",";
  for (auto& vi : f.vertices) out << " " << vi;
  // out << /* ", area: " << f.area << */ ", edges:";
  // for (auto& ei : f.edges) out << " " << ei;
  out << /* ", tetras: " << f.initial << " " << f.final << */ ")";
  return out;
}

Tetrahedron::Tetrahedron(int idx) : idx(idx) {}
Tetrahedron::Tetrahedron(int idx, int v0, int v1, int v2, int v3)
    : idx(idx), vertices({v0, v1, v2, v3}) {}
// Tetrahedron::Tetrahedron(const Tetrahedron& t) : idx(t.idx),
// vertices(t.vertices) {}

bool Tetrahedron::operator==(const Tetrahedron& o) const {
  return idx == o.idx && vertices[0] == o.vertices[0] &&
         vertices[1] == o.vertices[1] && vertices[2] == o.vertices[2] &&
         vertices[3] == o.vertices[3];
}

std::size_t Tetrahedron::Hash::operator()(const Tetrahedron& t) const {
  std::size_t seed = 0;
  std::hash<int> hasher;
  for (int vertex : t.vertices) {
    seed ^= hasher(vertex) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
  }
  return seed;
}

ostream& operator<<(ostream& out, const Tetrahedron& t) {
  out << "(Tetrahedron " << t.idx << ",";
  for (auto& vi : t.vertices) out << " " << vi;
  // out << ", edges:";
  // for (auto& ei : t.edges) out << " " << ei;
  // out << ", faces:";
  // for (auto& fi : t.faces) out << " " << fi;
  // out << ", neighs:";
  // for (auto& ti : t.neighs) out << " " << ti;
  out << ")";
  return out;
}

ostream& operator<<(ostream& out, const Polyhedron& p) {
  out << "(Polyhedron " << p.idx << ",";
  for (auto& vi : p.vertices) out << " " << vi;
  out << ")";
  return out;
}

Polyhedron::Polyhedron(int idx) : idx(idx) {}
Polyhedron::Polyhedron(int idx, const vector<int>& vertices)
    : idx(idx), vertices(vertices) {}

ostream& operator<<(ostream& out, const TetraMesh& m) {
  out << "MESH\n"
      << "    Total vertices: " << m.vertices.size() << "\n"
      << "    Total edges: " << m.vertices.size() << "\n"
      << "    Total faces: " << m.vertices.size() << "\n"
      << "    Total tetrahedrons: " << m.vertices.size() << "\n";
  out << "VERTICES\n";
  for (const auto& v : m.vertices) {
    out << "    " << v << "\n";
  }
  out << "EDGES\n";
  for (const auto& e : m.edges) {
    out << "    " << e << "\n";
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

ostream& operator<<(ostream& out, const PolyMesh& m) {
  out << "POLYMESH\n"
      << "    Total vertices: " << m.vertices.size() << "\n"
      << "    Total edges: " << m.vertices.size() << "\n"
      << "    Total faces: " << m.vertices.size() << "\n"
      << "    Total tetrahedrons: " << m.vertices.size() << "\n";
  out << "VERTICES\n";
  for (const auto& v : m.vertices) {
    out << "    " << v << "\n";
  }
  out << "EDGES\n";
  for (const auto& e : m.edges) {
    out << "    " << e << "\n";
  }
  out << "FACES\n";
  for (const auto& f : m.faces) {
    out << "    " << f << "\n";
  }
  out << "POLIHEDRAS\n";
  for (const auto& p : m.cells) {
    out << "    " << p << "\n";
  }
  return out;
}

}  // namespace gpolylla
