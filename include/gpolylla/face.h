#ifndef _GPOLYLLA_FACE_H_
#define _GPOLYLLA_FACE_H_
#include <gpolylla/edge.h>
#include <gpolylla/vertex.h>

#include <iostream>
#include <vector>

namespace gpolylla {
struct Tetrahedron;
struct Face {
  Tetrahedron *initial, *final;  // Tetras
  std::vector<Vertex*> vertices;
  std::vector<Edge*> edges;
  // std::vector<int> neighs;  // Tetras (captured in initial and final)

  inline bool isBoundary() const {
    return initial == nullptr || final == nullptr;
  }

  float area;
  friend std::ostream& operator<<(std::ostream&, const Face&);

 private:
  int idx;
};

}  // namespace gpolylla

#endif  // _GPOLYLLA_FACE_H_
