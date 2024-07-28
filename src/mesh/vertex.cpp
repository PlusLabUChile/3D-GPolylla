#include <gpolylla/vertex.h>
namespace gpolylla {
using std::ostream;
bool Vertex::operator==(const Vertex& o) const {
  return idx == o.idx && x == o.x && y == o.y && z == o.z;
}
ostream& operator<<(ostream& out, const Vertex& v) {
  out << "(Vertex " << v.idx << ", " << v.x << " " << v.y << " " << v.z << ")";
  return out;
}
} // gpolylla
