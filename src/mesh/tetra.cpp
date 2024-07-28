#include <gpolylla/tetra.h>

namespace gpolylla {
using std::ostream;
bool Tetrahedron::operator==(const Tetrahedron& o) const {
  return idx == o.idx && vertices == o.vertices;
}

ostream& operator<<(ostream& out, const Tetrahedron& t) {
  out << "(Tetrahedron " << t.idx << ",";
  for (auto& vi : t.vertices) out << " " << vi;
  // out << ", edges:";
  // for (auto& ei : t.edges) out << " " << ei;
  out << ", faces:";
  for (auto& fi : t.faces) out << " " << fi;
  out << ", neighs:";
  for (auto& ti : t.neighs) out << " " << ti;
  out << ")";
  return out;
}
}
