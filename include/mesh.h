#include <string>
#include <vector>

namespace GPolylla {

struct Polyhedron {
  std::vector<double> tetras;
  std::vector<int> faces;
};

struct Vertex {
  double x, y, z;
  int id;
};

struct Face {
  int id;
  int v1, v2, v3;
  bool is_boundary;
  int n1, n2;
  std::vector<int> edges;
  double area;
};

struct Tetrahedron {
  int id;
  int v1, v2, v3, v4;
  std::vector<int> neighs;
  bool is_boundary;
  std::vector<int> faces;
  std::vector<int> edges;
};

struct Edge {
  int id;
  int v1, v2;
  int first_tetra;
  bool is_boundary;
  std::vector<int> tetras;
  std::vector<int> faces;
  double length;
};

class TetrahedronMesh {
 private:
  std::vector<Vertex> nodes;
  std::vector<Face> faces;
  std::vector<Tetrahedron> tetras;
  std::vector<Edge> edges;
  void construct_tetrahedral_mesh(const std::string& node_filename,
                                  const std::string& face_filename,
                                  const std::string& ele_filename,
                                  const std::string& edge_filename);
  void read_node_file(const std::string& filename);
  void read_face_file(const std::string& filename);
  void read_ele_file(const std::string& filename);
  void read_edge_file(const std::string& filename);

 public:
  TetrahedronMesh(const std::string& node_filename,
                  const std::string& face_filename,
                  const std::string& ele_filename,
                  const std::string& edge_filename);
  Face& get_face(int id);
  Tetrahedron& get_tetra(int id);
  Edge& get_edge(int id);
  Vertex& get_vertex(int id);
  int num_faces();
  int num_tetrahedrons();
  int num_edges();
  int num_nodes();
  void display_info();
};
}  // namespace GPolylla
