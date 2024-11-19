#ifndef _GPOLYLLA_MESH_FACE_H_
#define _GPOLYLLA_MESH_FACE_H_
#include <gpolylla/mesh/basic.h>
#include <array>

namespace gpolylla {

class FaceVertex: public Vertex {
public:
  FaceVertex(const Vertex&);
};

class FaceFace: public Face {
public:
  std::array<int, 2> tetras;
  int fittest;
  FaceFace(const Face&);

  friend std::ostream& operator<<(std::ostream&, const FaceFace&);
};

class FaceTetrahedron: public Tetrahedron {
public:
  int fittest;
  std::array<int, 4> faces;
  FaceTetrahedron(const Tetrahedron&);

  friend std::ostream& operator<<(std::ostream& out, const FaceTetrahedron& t);
};

class FaceTetraMesh {
public:
  std::vector<FaceVertex> vertices;
  // std::vector<Edge> edges;
  std::vector<FaceFace> faces;
  std::vector<FaceTetrahedron> tetras;

  FaceTetraMesh() = default;
  FaceTetraMesh(const TetraMesh& m);

  friend std::ostream& operator<<(std::ostream& out, const FaceTetraMesh& m);
};
}  // namespace gpolylla

#endif  // _GPOLYLLA_MESH_FACE_H_
