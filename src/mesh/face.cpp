#include <gpolylla/face.h>
namespace gpolylla {
using std::ostream;
ostream& operator<<(ostream& out, const Face& f) {
  out << "(Face " << f.idx << ",";
  for (auto& vi : f.vertices) out << " " << vi;
  out << ", area: " << f.area << ", edges:";
  for (auto& ei : f.edges) out << " " << ei;
  out << ", tetras: " << f.initial << " " << f.final << ")";
  return out;
}
}
