#ifndef _GPOLYLLA_TETRA_H_
#define _GPOLYLLA_TETRA_H_
#include <gpolylla/vertex.h>
#include <gpolylla/face.h>
#include <gpolylla/edge.h>

#include <iostream>
#include <vector>

namespace gpolylla {
struct Tetrahedron {
  std::vector<Vertex*> vertices;
  std::vector<Tetrahedron*> neighs;
  std::vector<Face*> faces;
  std::vector<Edge*> edges;
  bool isBoundary;
  bool operator==(const Tetrahedron&) const;
  friend std::ostream& operator<<(std::ostream& out, const Tetrahedron& t);

 private:
  int idx;
};

}  // namespace gpolylla
#endif  // _GPOLYLLA_TETRA_H_
