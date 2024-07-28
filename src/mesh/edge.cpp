#include <gpolylla/edge.h>

namespace gpolylla {
using std::ostream;
ostream& operator<<(ostream& out, const Edge& e) {
  out << "(Edge " << e.idx << ", " << e.initial << " " << e.final
      << ", len: " << e.length << ", faces:";
  for (auto& fi : e.faces) out << " " << fi;
  out << ")";
  return out;
}
}
