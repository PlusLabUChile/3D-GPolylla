#ifndef GPOLYLLA_MESH_H
#define GPOLYLLA_MESH_H
#include <string>
#include <vector>

namespace gpolylla {
struct Vertex {
  int idx;
  float x, y, z;
  bool operator==(const Vertex&) const;
  friend std::ostream& operator<<(std::ostream&, const Vertex&);
};
// bool operator==(const Vertex& l, const Vertex& r) {
//   return l.idx == r.idx;
// }

struct Edge {
  int idx;
  int initial, final;  // Vertices
  std::vector<int> faces;
  float length;
  friend std::ostream& operator<<(std::ostream&, const Edge&);
};

struct Face {
  int idx;
  int initial, final;  // Tetras
  std::vector<int> vertices;
  std::vector<int> edges;
  // std::vector<int> neighs;  // Tetras (captured in initial and final)

  inline bool isBoundary() const { return initial == -1 || final == -1; }
  
  float area;
  friend std::ostream& operator<<(std::ostream&, const Face&);
};

struct Tetrahedron {
  int idx;
  std::vector<int> vertices;
  std::vector<int> neighs;
  std::vector<int> faces;
  std::vector<int> edges;
  bool isBoundary;
  bool operator==(const Tetrahedron&) const;
  friend std::ostream& operator<<(std::ostream& out, const Tetrahedron& t);
};

class Mesh {
 public:
  std::vector<Vertex> nodes;
  std::vector<Face> faces;
  std::vector<Tetrahedron> tetras;
  std::vector<Edge> edges;

 public:
  Mesh(const std::string& node_filename, const std::string& face_filename,
       const std::string& ele_filename);

  friend std::ostream& operator<<(std::ostream& out, const Mesh& m);
};

}  // namespace gpolylla


#endif  // GPOLYLLA_MESH_H
