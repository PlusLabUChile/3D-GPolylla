#ifndef INCLUDE_GPOLYLLA_MESH_H_
#define INCLUDE_GPOLYLLA_MESH_H_

#include <cstddef>
#include <functional>
#include <string>
#include <vector>

namespace gpolylla {
struct Face;
struct Tetrahedron;

struct Vertex {
  Vertex() = default;
  explicit Vertex(int idx) : idx_(idx) {}
  double x, y, z;
  bool operator==(const Vertex &other) const;
  friend std::ostream &operator<<(std::ostream &os, const Vertex &f);

 private:
  int idx_;
};

struct Edge {
  Vertex *initial, *final;
  std::vector<Face *> faces;
  double length;

  bool operator==(const Edge &other) const;
  friend std::ostream &operator<<(std::ostream &os, const Edge &f);
};

struct Face {
  Face() = default;
  explicit Face(int idx) : idx_(idx) {}
  Tetrahedron *init, *final;
  std::vector<Vertex *> vertices;

  bool operator==(const Face &other) const;
  friend std::ostream &operator<<(std::ostream &os, const Face &f);

 private:
  int idx_;
};

struct Tetrahedron {
  Tetrahedron() = default;
  explicit Tetrahedron(int idx) : idx_(idx) {}
  std::vector<Vertex *> vertices;
  std::vector<Face *> faces;
  std::vector<Tetrahedron *> neighbours;

  bool operator==(const Tetrahedron &other) const;

 private:
  int idx_;
};

class TetrahedronMesh {
 public:
  TetrahedronMesh() = default;
  TetrahedronMesh(const std::string &node_filename,
                  const std::string &ele_filename);
  const Vertex &get_vertex(std::size_t idx) const;
  const Edge &get_edge(std::size_t idx) const;
  const Face &get_face(std::size_t idx) const;
  const Tetrahedron &get_tetrahedron(std::size_t idx) const;
  std::size_t num_faces() const;
  std::size_t num_tetrahedrons() const;
  std::size_t num_edges() const;
  std::size_t num_vetices() const;
  void write_off(const std::string &filename) const;

 private:
  std::vector<Vertex> vertices_;
  std::vector<Edge> edges_;
  std::vector<Face> faces_;
  std::vector<Tetrahedron> tetrahedrons_;
};
}  // namespace gpolylla

namespace std {

template <>
struct hash<gpolylla::Vertex> {
  size_t operator()(const gpolylla::Vertex &v) const noexcept {
    size_t h1 = hash<double>()(v.x);
    size_t h2 = hash<double>()(v.y);
    size_t h3 = hash<double>()(v.z);
    return h1 ^ (h2 << 1) ^ (h3 << 2);
  }
};

template <>
struct hash<gpolylla::Edge> {
  size_t operator()(const gpolylla::Edge &e) const noexcept {
    size_t h1 = hash<gpolylla::Vertex>()(*e.initial);
    size_t h2 = hash<gpolylla::Vertex>()(*e.final);
    return h1 ^ (h2 << 1);
  }
};

template <>
struct hash<gpolylla::Face> {
  size_t operator()(const gpolylla::Face &f) const noexcept {
    size_t h = 0;
    for (int i = 0; i < f.vertices.size(); i++) {
      const auto &v = *f.vertices[i];
      h ^= hash<gpolylla::Vertex>()(v);
    }
    return h;
  }
};

}  // namespace std

#endif  // INCLUDE_GPOLYLLA_MESH_H_
