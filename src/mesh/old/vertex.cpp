#include <gpolylla/mesh/vertex.h>
namespace gpolylla {
using std::ostream;

Vertex::Vertex(int idx) : idx(idx) {}

Vertex::Vertex(const Vertex& v) : idx(v.idx), x(v.x), y(v.y), z(v.z) {}

bool Vertex::operator==(const Vertex& o) const {
  return idx == o.idx && x == o.x && y == o.y && z == o.z;
}

std::size_t Vertex::Hash::operator()(const Vertex& v) const {
  return std::hash<int>()(v.idx);
}

ostream& operator<<(ostream& out, const Vertex& v) {
  out << "(Vertex " << v.idx << ", " << v.x << " " << v.y << " " << v.z << ")";
  return out;
}

}  // namespace gpolylla
