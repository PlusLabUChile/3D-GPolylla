#ifndef INCLUDE_GPOLYLLA_MESH_H_
#define INCLUDE_GPOLYLLA_MESH_H_

#include <cstddef>
#include <string>
#include <vector>

namespace gpolylla {
struct Face;
struct Tetrahedron;

struct Vertex {
  double x, y, z;
};

struct Edge {
  Vertex *initial, *final;
  std::vector<Face *> faces;
  double length;
};

struct Face {
  Tetrahedron *init, *final;
  std::vector<Vertex *> vertices;
  std::vector<Tetrahedron *> neighbours;
};

struct Tetrahedron {
  std::vector<Vertex *> vertices;
  std::vector<Tetrahedron *> neighbours;
  std::vector<Face *> faces;
};

class TetrahedronMesh {
 public:
  TetrahedronMesh(const std::string &node_filename,
                  const std::string &face_filename,
                  const std::string &ele_filename);
  const Vertex& get_vertex(std::size_t idx) const;
  const Edge& get_edge(std::size_t idx) const;
  const Face& get_face(std::size_t idx) const;
  const Tetrahedron& get_tetrahedron(std::size_t idx) const;
  std::size_t num_faces() const;
  std::size_t num_tetrahedrons() const;
  std::size_t num_edges() const;
  std::size_t num_vetices() const;
  void write_off(const std::string& filename) const;

 private:
  std::vector<Vertex> vertices;
  std::vector<Edge> edges;
  std::vector<Face> faces;
  std::vector<Tetrahedron> tetrahedrons;
};
}


#endif  // INCLUDE_GPOLYLLA_MESH_H_
