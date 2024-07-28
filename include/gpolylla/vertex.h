#ifndef _GPOLYLLA_VERTEX_H_
#define _GPOLYLLA_VERTEX_H_
#include <iostream>

namespace gpolylla {
class Vertex {
 public:
  // int idx;
  float x, y, z;
  Vertex(int idx, float x, float y, float z);
  bool operator==(const Vertex&) const;
  friend std::ostream& operator<<(std::ostream&, const Vertex&);
  
  struct Hash {
    std::size_t operator()(const Vertex& v) const;
  };

 private:
  int idx;
};
}  // namespace gpolylla
#endif  // _GPOLYLLA_VERTEX_H_
