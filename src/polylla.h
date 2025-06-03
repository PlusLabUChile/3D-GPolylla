#ifndef POLYLLA_H
#define POLYLLA_H
#include <Eigen/Dense>

namespace Polylla {

class Vertex : public Eigen::Vector3f {
  using Eigen::Vector3f::Vector3f;
};

class Face {
 public:
  Face(int v1, int v2, int v3);

  int operator[](int index) const;
  int size() const;

 private:
  Eigen::Array3i vertices;
};

class Tetrahedron {
 public:
  Tetrahedron(int v1, int v2, int v3, int v4);

  int operator[](int index) const;
  int size() const;

 private:
  Eigen::Array4i vertices;
};

class Polyhedron {
 public:
 private:
  Eigen::ArrayXi vertices;
};

class Mesh {
 public:
  Mesh(const Eigen::ArrayX<Vertex>& vertices, const Eigen::ArrayX<Face>& faces,
       const Eigen::ArrayX<Tetrahedron>& cells);

 private:
  Eigen::ArrayX<Vertex> vertices;
  Eigen::ArrayX<Face> faces;
  Eigen::ArrayX<Tetrahedron> cells;
};

class PolyMesh {
 public:
 private:
  Eigen::ArrayX<Vertex> vertices;
  Eigen::ArrayX<Face> faces;
  Eigen::ArrayX<Polyhedron> cells;
};

class Polylla {
  virtual PolyMesh operator()(const Mesh& mesh) = 0;
  virtual ~Polylla() = 0;
};

class CavityPolylla : public Polylla {
 public:
  PolyMesh operator()(const Mesh& mesh) override;
};

class Reader {
  virtual Mesh read(const std::string& filename) = 0;
  virtual ~Reader() = 0;
};

class Writer {
 public:
  virtual void write(const PolyMesh& mesh, const std::string& filename) = 0;
  virtual ~Writer() = 0;
};

class TetgenReader : public Reader {
 public:
  Mesh read(const std::string& filename) override;
};

class VisfWriter : public Writer {
 public:
  void write(const PolyMesh& mesh, const std::string& filename) override;
};

}  // namespace Polylla

#endif  // POLYLLA_H
