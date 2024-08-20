#include <gpolylla/mesh/tetra.h>
#include <cstddef>

namespace gpolylla {
using std::ostream;

Tetrahedron::Tetrahedron(int idx) : idx(idx) {}

Tetrahedron::Tetrahedron(const Tetrahedron& t)
    : idx(t.idx), vertices(t.vertices) {}

bool Tetrahedron::operator==(const Tetrahedron& o) const {
  return idx == o.idx && *vertices[0] == *o.vertices[0] &&
         *vertices[1] == *o.vertices[1] && *vertices[2] == *o.vertices[2] &&
         *vertices[3] == *o.vertices[3];
}

std::size_t Tetrahedron::Hash::operator()(const Tetrahedron& t) const {
  return std::hash<int>()(t.idx);
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
}  // namespace gpolylla
