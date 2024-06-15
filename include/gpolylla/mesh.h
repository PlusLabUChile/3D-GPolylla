#include <string>
#include <vector>

namespace gpolylla {
struct Vertex {
  int idx;
  float x, y, z;
};

struct Edge {
  int idx;
  int initial, final;  // Vertices
  std::vector<int> faces;
  float length;
};

struct Face {
  int idx;
  int initial, final;  // Tetras
  std::vector<int> vertices;
  std::vector<int> edges;
  std::vector<int> neighs;  // Faces
  bool isBoundary;
  float area;
};

struct Tetrahedron {
  int idx;
  std::vector<int> vertices;
  std::vector<int> neighs;
  std::vector<int> faces;
  std::vector<int> edges;
  bool isBoundary;
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
};

}  // namespace gpolylla
