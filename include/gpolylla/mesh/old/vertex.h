#ifndef _GPOLYLLA_MESH_VERTEX_H_
#define _GPOLYLLA_MESH_VERTEX_H_
#include <iostream>

namespace gpolylla {
class Vertex {
 public:
  float x, y, z;

  Vertex(int idx);
  Vertex(const Vertex& v);

  bool operator==(const Vertex&) const;
  
  struct Hash {
    std::size_t operator()(const Vertex& v) const;
  };

  friend std::ostream& operator<<(std::ostream&, const Vertex&);
 private:
  int idx; // used for hashing
};
}  // namespace gpolylla
#endif  // _GPOLYLLA_MESH_VERTEX_H_
