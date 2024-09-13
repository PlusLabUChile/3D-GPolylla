#include <gpolylla/mesh/face.h>

namespace gpolylla {
using std::ostream;

Face::Face(int idx) : idx(idx) {}

Face::Face(const Face& f) : idx(f.idx), vertices(f.vertices) {}

bool Face::operator==(const Face& f) const {
  return idx == f.idx && *vertices[0] == *f.vertices[0] && *vertices[1] == *f.vertices[1] &&
         *vertices[2] == *f.vertices[2];
}

std::size_t Face::Hash::operator()(const Face& f) const {
  return std::hash<int>()(f.idx);
}

ostream& operator<<(ostream& out, const Face& f) {
  out << "(Face " << f.idx << ",";
  for (auto& vi : f.vertices) out << " " << vi;
  // out << /* ", area: " << f.area << */ ", edges:";
  // for (auto& ei : f.edges) out << " " << ei;
  out << /* ", tetras: " << f.initial << " " << f.final << */ ")";
  return out;
}

}  // namespace gpolylla
