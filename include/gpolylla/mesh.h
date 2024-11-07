#ifndef _GPOLYLLA_MESH_H_
#define _GPOLYLLA_MESH_H_
#include <array>
#include <iostream>
#include <vector>
#include <Eigen/Dense>
#include "Eigen/Core"

namespace gpolylla {
class Vertex {
 public:
  int idx;
  float x, y, z;

  Vertex(int idx);
  Vertex(int idx, float x, float y, float z);
  Vertex(const Vertex& v);

  bool operator<(const Vertex&) const;
  bool operator==(const Vertex&) const;

  operator Eigen::Vector3d() const;

  struct Hash {
    std::size_t operator()(const Vertex& v) const;
  };

  friend std::ostream& operator<<(std::ostream&, const Vertex&);
};

class Edge {
 public:
  int idx;
  std::array<int, 2> vertices;

  Edge(int idx);
  Edge(int idx, int v0, int v1);
  Edge(const Edge& e);

  bool operator==(const Edge&) const;

  struct Hash {
    std::size_t operator()(const Edge& e) const;
  };

  friend std::ostream& operator<<(std::ostream&, const Edge&);
};

class Face {
 public:
  int idx;
  std::array<int, 3> vertices;

  Face(int idx);
  Face(int idx, int v0, int v1, int v2);
  // Face(const Face& f);

  bool operator==(const Face&) const;

  struct Hash {
    std::size_t operator()(const Face& f) const;
  };

  friend std::ostream& operator<<(std::ostream&, const Face&);
};

class Tetrahedron {
 public:
  int idx;
  std::array<int, 4> vertices;

  Tetrahedron(int idx);
  Tetrahedron(int idx, int v0, int v1, int v2, int v3);
  // Tetrahedron(const Tetrahedron& t);

  bool operator==(const Tetrahedron& t) const;
  inline std::size_t size() const { return vertices.size();}
  inline int operator[](int idx) const { return vertices[idx];}

  struct Hash {
    std::size_t operator()(const Tetrahedron& t) const;
  };

  friend std::ostream& operator<<(std::ostream& out, const Tetrahedron& t);
};

class Polyhedron {
 public:
  int idx;
  std::vector<int> vertices;
  std::vector<int> tetras;

  Polyhedron(int idx);
  Polyhedron(int idx, const std::vector<int>& vertices);

  friend std::ostream& operator<<(std::ostream& out, const Polyhedron& p);
};

class TetraMesh {
 public:
  std::vector<Vertex> vertices;
  std::vector<Edge> edges;
  std::vector<Face> faces;
  std::vector<Tetrahedron> tetras;

  friend std::ostream& operator<<(std::ostream& out, const TetraMesh& m);
};

class PolyMesh {
 public:
  std::vector<Vertex> vertices;
  std::vector<Edge> edges;
  std::vector<Face> faces;
  std::vector<Tetrahedron> tetras;
  std::vector<Polyhedron> cells;

  friend std::ostream& operator<<(std::ostream& out, const PolyMesh& m);
};


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


class CavityVertex: public Vertex {
public:
  CavityVertex(const Vertex& v);

};

class CavityFace: public Face {
public:
  CavityFace(const Face& f);
};

class CavityTetrahedron: public Tetrahedron {
public:
  CavityTetrahedron(const Tetrahedron& t);

  // std::vector<int> fittest;
  Eigen::Vector3d fittest;
  std::array<int, 4> neighs;
};

class CavityTetraMesh {
public:
  std::vector<CavityVertex> vertices;
  std::vector<CavityFace> faces;
  std::vector<CavityTetrahedron> tetras;

  CavityTetraMesh() = default;
  CavityTetraMesh(const TetraMesh& m);

  friend std::ostream& operator<<(std::ostream& out, const CavityTetraMesh& m);
};

}  // namespace gpolylla
#endif  // _GPOLYLLA_MESH_BASIC_MESH_H
