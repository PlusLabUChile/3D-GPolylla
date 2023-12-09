#ifndef GPOL_MESH_H
#define GPOL_MESH_H
#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace GPolylla {

class Polyhedron {
 public:
  std::vector<int> tetras;
  std::vector<int> faces;
  bool was_repaired;

 public:
  Polyhedron();
};

class Vertex {
 public:
  double x, y, z;
  int id;

 public:
  Vertex();
  friend std::ostream& operator<<(std::ostream& os, const Vertex& v);
};

class Face {
 public:
  int id;
  int v1, v2, v3;
  int n1, n2;
  std::vector<int> edges;
  std::vector<int> neighs;
  bool is_boundary;

 public:
  Face();
  friend std::ostream& operator<<(std::ostream& os, const Face& f);
};

class Edge {
 public:
  int id;
  int vi, vf;
  std::vector<int> faces;
  int first_tetra;

 public:
  Edge();
  bool compare(int v1, int v2) const;
  friend std::ostream& operator<<(std::ostream& os, const Edge& e);
};

class Tetrahedron {
 public:
  int id;
  int v1, v2, v3, v4;
  std::vector<int> neighs;
  bool is_boundary;
  std::vector<int> faces;
  std::vector<int> edges;

 public:
  Tetrahedron();
  friend std::ostream& operator<<(std::ostream& os, const Tetrahedron& t);
};

class TetrahedronMesh {
 private:
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
  int num_faces() const;
  int num_tetrahedrons() const;
  int num_edges() const;
  int num_nodes() const;
  void display_info() const;

 private:
  void construct_tetrahedral_mesh(const std::string& node_filename,
                                  const std::string& face_filename,
                                  const std::string& ele_filename);
  void read_node_file(
      const std::string& filename,
      std::vector<std::vector<int>>* edges_matrix_vertex,
      std::vector<std::vector<std::vector<int>>>* edges_matrix_faces);
  void read_face_file(
      const std::string& filename,
      std::vector<std::vector<int>>* edges_matrix_vertex,
      std::vector<std::vector<std::vector<int>>>* edges_matrix_faces);
  void read_ele_file(const std::string& filename);
  void read_edge_file(
      std::vector<std::vector<int>>* edges_matrix_vertex,
      std::vector<std::vector<std::vector<int>>>* edges_matrix_faces);
  void asign_faces_to_tetras();
  void asign_neighs(Tetrahedron* t);
};
}  // namespace GPolylla
#endif
