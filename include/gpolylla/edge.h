#ifndef _GPOLYLLA_EDGE_H_
#define _GPOLYLLA_EDGE_H_
#include <gpolylla/vertex.h>

#include <iostream>
#include <vector>

namespace gpolylla {
struct Face;
struct Edge {
  Vertex *initial, *final;  // Vertices
  std::vector<Face*> faces;
  float length;
  friend std::ostream& operator<<(std::ostream&, const Edge&);

 private:
  int idx;
};

}  // namespace gpolylla

#endif  // _GPOLYLLA_EDGE_H_
