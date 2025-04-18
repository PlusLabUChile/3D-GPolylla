#ifndef _GPOLYLLA_MESH_EDGE_H_
#define _GPOLYLLA_MESH_EDGE_H_
#include <gpolylla/mesh/vertex.h>

#include <iostream>
#include <array>

namespace gpolylla {
struct Edge {
  std::array<Vertex*, 2> vertices;
  
  Edge(int idx);
  Edge(const Edge& e);

  bool operator==(const Edge&) const;
  
  struct Hash {
    std::size_t operator()(const Edge& e) const;
  };

  friend std::ostream& operator<<(std::ostream&, const Edge&);

 private:
  const int idx;
};

}  // namespace gpolylla

#endif  // _GPOLYLLA_EDGE_H_
