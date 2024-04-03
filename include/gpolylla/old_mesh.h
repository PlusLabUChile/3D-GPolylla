#ifndef INCLUDE_GPOLYLLA_MESH_H_
#define INCLUDE_GPOLYLLA_MESH_H_
#include <iostream>
#include <string>
#include <vector>

namespace GPolylla {

struct Polyhedron {
  std::vector<int> tetrahedrons;
  std::vector<int> faces;
  bool was_repaired;
};

struct Vertex {
  int idx;
  double x, y, z;  // coordinates

  friend std::ostream& operator<<(std::ostream& os, const Vertex& v);
};

struct Face {
  int idx;
  int v1, v2, v3;           // 3 vertex makes a triangle
  int ni, nf;               // init tetra to final tetra
  std::vector<int> edges;   // edges (maybe 3?)
  std::vector<int> neighs;  // faces neighs (unnecesary?)
  bool is_boundary;

  // Face();
  friend std::ostream& operator<<(std::ostream& os, const Face& f);
};

struct Edge {
  // public:
  int idx;
  int vi, vf;              // init vertex to final vertex
  std::vector<int> faces;  // faces
  // int first_tetra;         // idk
  double length;

  // public:
  // Edge();
  // bool compare(int v1, int v2) const;
  friend std::ostream& operator<<(std::ostream& os, const Edge& e);
};

struct Tetrahedron {
  // public:
  int idx;
  int v1, v2, v3, v4;
  bool is_boundary;
  std::vector<int> neighs;  // not necessary??
  std::vector<int> faces;
  std::vector<int> edges;

  // public :
  //  Tetrahedron();
  friend std::ostream& operator<<(std::ostream& os, const Tetrahedron& t);
};

class TetrahedronMesh {
 public:
  std::vector<Vertex> nodes;
  std::vector<Face> faces;
  std::vector<Tetrahedron> tetras;
  std::vector<Edge> edges;

 public:
  TetrahedronMesh(const std::string& node_filename,
                  const std::string& face_filename,
                  const std::string& ele_filename);
  Face& get_face(int id);
  Tetrahedron& get_tetra(int id);
  Edge& get_edge(int id);
  Vertex& get_vertex(int id);
  std::vector<std::vector<int>> get_faces() const;
  std::vector<std::vector<int>> get_tetrahedrons() const;
  std::vector<std::vector<double>> get_vertices() const;
  int num_faces() const;
  int num_tetrahedrons() const;
  int num_edges() const;
  int num_nodes() const;
  void display_info() const;
  void write_off(const std::string& filename) const;

 private:
  void construct_tetrahedral_mesh(const std::string& node_filename,
                                  const std::string& face_filename,
                                  const std::string& ele_filename);

  // void read_node_file(
  //     const std::string& filename,
  //     std::vector<std::vector<int>>* edges_matrix_vertex,
  //     std::vector<std::vector<std::vector<int>>>* edges_matrix_faces);
  // void read_face_file(
  //     const std::string& filename,
  //     std::vector<std::vector<int>>* edges_matrix_vertex,
  //     std::vector<std::vector<std::vector<int>>>* edges_matrix_faces);
  // void read_ele_file(const std::string& filename);
  // void read_edge_file(
  //     std::vector<std::vector<int>>* edges_matrix_vertex,
  //     std::vector<std::vector<std::vector<int>>>* edges_matrix_faces);
  // void asign_faces_to_tetras();
  // void asign_neighs(Tetrahedron* t);
};
}  // namespace GPolylla
#endif  // INCLUDE_GPOLYLLA_MESH_H_
