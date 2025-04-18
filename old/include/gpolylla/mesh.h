#ifndef _GPOLYLLA_MESH_H_
#define _GPOLYLLA_MESH_H_

#include <gpolylla/utils.h>

#include <Eigen/Dense>
#include <array>
#include <iostream>
#include <vector>

namespace gpolylla {
class Vertex {
 public:
  float x, y, z;

  Vertex();

  Vertex(float x, float y, float z);

  Vertex(const Vertex &v);

  // bool operator<(const Vertex&) const;
  bool operator==(const Vertex &) const;
  //
  operator Eigen::Vector3f() const;

  struct Hash {
    std::size_t operator()(const Vertex &v) const;
  };

  friend std::ostream &operator<<(std::ostream &, const Vertex &);
};

// class Edge {
//  public:
//   int tetra;
//
//   Edge();
//
//   Edge(int v0, int v1);
//
//   Edge(const Edge &e);
//
//   bool operator==(const Edge &) const;
//
//   static std::size_t size() { return 2; }
//
//   int operator[](int idx) const;
//   int &operator[](int idx);
//
//   struct Hash {
//     std::size_t operator()(const Edge &e) const;
//   };
//
//   friend std::ostream &operator<<(std::ostream &, const Edge &);
//
//  private:
//   std::array<int, 2> vertices;
// };

class Face {
 public:
  // int tetra;

  Face();

  Face(int v0, int v1, int v2);

  Face(const Face &f);

  bool operator==(const Face &) const;

  static std::size_t size() { return 3; }

  int operator[](int idx) const;
  int &operator[](int idx);

  struct Hash {
    std::size_t operator()(const Face &f) const;
  };

  friend std::ostream &operator<<(std::ostream &, const Face &);

 private:
  std::array<int, 3> vertices;
};

class Tetrahedron {
 public:
  // std::array<int, 4> faces;

  Tetrahedron();

  Tetrahedron(int v0, int v1, int v2, int v3);

  Tetrahedron(const Tetrahedron &t);

  bool operator==(const Tetrahedron &t) const;

  static std::size_t size() { return 4; }

  int operator[](int idx) const;
  int &operator[](int idx);

  // int face(int idx) const;
  // int &face(int idx);

  struct Hash {
    std::size_t operator()(const Tetrahedron &t) const;
  };

  friend std::ostream &operator<<(std::ostream &out, const Tetrahedron &t);

 private:
  std::array<int, 4> vertices;
  // int faces[4];
};

class Polyhedron {
 public:
  //  std::vector<int> tetras;
  // std::vector<int> faces;

  Polyhedron() = default;
  Polyhedron(const Polyhedron &p) = default;
  // Polyhedron(int idx, const std::vector<int>& faces);

  bool operator==(const Polyhedron &p) const;

  std::size_t numVertices() const { return vertices.size(); }
  std::size_t numFaces() const { return faces.size(); }
  std::size_t numTetras() const { return tetras.size(); }

  int face(int idx) const { return vertices.at(idx); }
  int &face(int idx) { return vertices.at(idx); }
  int vertex(int idx) const { return vertices.at(idx); }
  int &vertex(int idx) { return vertices.at(idx); }
  int tetra(int idx) const { return tetras.at(idx); }
  int &tetra(int idx) { return tetras.at(idx); }

  struct Hash {
    std::size_t operator()(const Polyhedron &p) const;
  };

  friend std::ostream &operator<<(std::ostream &out, const Polyhedron &p);

 private:
  std::vector<int> vertices;
  std::vector<int> tetras;
  std::vector<int> faces;
};

class Mesh {};

class TetraMesh {
 public:
  std::vector<Vertex> vertices;
  std::vector<Tetrahedron> tetras;

  // Tetrahedron tetra(const Vertex &v) const { return tetras[v.tetra]; }
  // Tetrahedron tetra(const Edge &e) const { return tetras[e.tetra]; }
  // Tetrahedron tetra(const Face &f) const { return tetras[f.tetra]; }

  friend std::ostream &operator<<(std::ostream &out, const TetraMesh &m);
};

class PolyMesh : public Mesh {
 public:
  std::vector<Vertex> vertices;
  std::vector<Face> faces;
  std::vector<Tetrahedron> tetras;
  std::vector<Polyhedron> cells;

  friend std::ostream &operator<<(std::ostream &out, const PolyMesh &m);
};

class FaceFace : public Face {
 public:
  //        std::array<int, 2> tetras;
  //        int fittest;
  int twin;

  FaceFace(const Face &);

  friend std::ostream &operator<<(std::ostream &, const FaceFace &);
};

class FaceTetraMesh {
 public:
  std::vector<Vertex> vertices;
  std::vector<FaceFace> faces;
  std::vector<Tetrahedron> tetras;

  FaceTetraMesh() = default;

  FaceTetraMesh(const TetraMesh &m);

  friend std::ostream &operator<<(std::ostream &out, const FaceTetraMesh &m);
};

class CavityTetrahedron : public Tetrahedron {
 public:
  CavityTetrahedron(const Tetrahedron &t);

  std::array<int, 4> neighs;
};

class CavityTetraMesh : public TetraMesh {
 public:
  std::vector<Face> faces;
  CavityTetraMesh() = default;

  CavityTetraMesh(const TetraMesh &m);

  friend std::ostream &operator<<(std::ostream &out, const CavityTetraMesh &m);
};

}  // namespace gpolylla
#endif  // _GPOLYLLA_MESH_BASIC_MESH_H
