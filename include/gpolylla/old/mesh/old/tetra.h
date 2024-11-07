#ifndef _GPOLYLLA_TETRA_H_
#define _GPOLYLLA_TETRA_H_
// #include <gpolylla/mesh/edge.h>
// #include <gpolylla/mesh/face.h>
#include <gpolylla/mesh/vertex.h>

#include <iostream>
#include <array>

namespace gpolylla {
class Tetrahedron {
 public:
  std::array<Vertex*, 4> vertices;

  Tetrahedron(int idx);
  Tetrahedron(const Tetrahedron& t);

  bool operator==(const Tetrahedron& t) const;

  struct Hash {
    std::size_t operator()(const Tetrahedron& t) const;
  };

  friend std::ostream& operator<<(std::ostream& out, const Tetrahedron& t);

  // std::vector<Tetrahedron*> neighs;
  // std::vector<Face*> faces;
  // std::vector<Edge*> edges;
  // bool isBoundary;
 private:
  const int idx;
};

}  // namespace gpolylla
#endif  // _GPOLYLLA_TETRA_H_
