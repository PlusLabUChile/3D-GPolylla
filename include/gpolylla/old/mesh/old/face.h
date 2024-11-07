#ifndef _GPOLYLLA_FACE_H_
#define _GPOLYLLA_FACE_H_
#include <gpolylla/mesh/edge.h>
#include <gpolylla/mesh/vertex.h>

#include <iostream>
#include <array>

namespace gpolylla {
class Face {
 public:
  std::array<Vertex*, 3> vertices;

  Face(int idx);
  Face(const Face& f);

  bool operator==(const Face&) const;

  struct Hash {
    std::size_t operator()(const Face& f) const;
  };

  friend std::ostream& operator<<(std::ostream&, const Face&);

  // Tetrahedron *initial, *final;  // Tetras
  // std::vector<Edge*> edges;
  // std::vector<int> neighs;  // Tetras (captured in initial and final)
  //
  // inline bool isBoundary() const {
  //   return initial == nullptr || final == nullptr;
  // }
  //
  // float area;


 private:
  const int idx;
};

}  // namespace gpolylla

#endif  // _GPOLYLLA_FACE_H_
