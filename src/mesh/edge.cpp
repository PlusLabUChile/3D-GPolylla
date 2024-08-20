#include <gpolylla/mesh/edge.h>

namespace gpolylla {
using std::ostream;

Edge::Edge(int idx) : idx(idx) {}
Edge::Edge(const Edge& e) : idx(e.idx), vertices(e.vertices) {}

bool Edge::operator==(const Edge& e) const {
  return idx == e.idx && *vertices[0] == *e.vertices[0] &&
         *vertices[1] == *e.vertices[1];
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
}  // namespace gpolylla
