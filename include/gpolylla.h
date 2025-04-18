#ifndef GPOLYLLA_H
#define GPOLYLLA_H
#include <array>
#include <string>
#include <vector>

namespace gpolylla {

// DATA STRUCTURES
class Vertex {
 public:
  float x, y, z;

  Vertex() = default;
  Vertex(float x, float y, float z) : x(x), y(y), z(z) {};
};

class Face {
 public:
  std::array<int, 3> vertices;
  int tetra;

  Face() : Face({-1, -1, -1}) {};
  Face(const std::array<int, 3> &vertices) : Face(vertices, -1) {};
  Face(const std::array<int, 3> &vertices, int t)
      : vertices(vertices), tetra(t) {};
};

class Tetrahedron {
 public:
  std::array<int, 4> vertices;
  std::array<int, 4> faces;

  Tetrahedron() : Tetrahedron({-1, -1, -1, -1}) {};
  Tetrahedron(const std::array<int, 4> &vertices)
      : Tetrahedron(vertices, {-1, -1, -1, -1}) {};
  Tetrahedron(const std::array<int, 4> &vertices,
              const std::array<int, 4> &faces)
      : vertices(vertices), faces(faces) {};
};

class CavityTetrahedron {
 public:
  std::array<int, 4> vertices;
  std::array<int, 4> faces;
  std::array<int, 4> neighbours;

  CavityTetrahedron() : CavityTetrahedron({-1, -1, -1, -1}) {};
  CavityTetrahedron(const std::array<int, 4> &vertices)
      : CavityTetrahedron(vertices, {-1, -1, -1, -1}) {};
  CavityTetrahedron(const std::array<int, 4> &vertices,
                    const std::array<int, 4> &faces)
      : CavityTetrahedron(vertices, faces, {-1, -1, -1, -1}) {};
  CavityTetrahedron(const std::array<int, 4> &vertices,
                    const std::array<int, 4> &faces,
                    const std::array<int, 4> &neighbours)
      : vertices(vertices), faces(faces), neighbours(neighbours) {};
};

class Polyhedron {
 public:
  std::vector<int> tetras;
  std::vector<int> faces;
  std::vector<int> vertices;

  Polyhedron() = default;
  Polyhedron(const std::vector<int> &vertices, const std::vector<int> &faces,
             const std::vector<int> &tetras)
      : tetras(tetras), faces(faces), vertices(vertices) {};
};

class Mesh {
 public:
  Face &faceOfTetra(int ti, int fi) {
    return faces.at(tetras.at(ti).faces[fi]);
  };

  const Face &faceOfTetra(int ti, int fi) const {
    return faces.at(tetras.at(ti).faces[fi]);
  };

  Tetrahedron &tetraOfFace(int fi) { return tetras.at(faces.at(fi).tetra); }

  const Tetrahedron &tetraOfFace(int fi) const {
    return tetras.at(faces.at(fi).tetra);
  }

  std::vector<Vertex> vertices;
  std::vector<Face> faces;
  std::vector<Tetrahedron> tetras;
};

class CavityMesh {
 public:
  CavityMesh(const Mesh &m);

  [[nodiscard]] const Vertex &vertexOfTetra(int ti, int vi) const {
    return vertices.at(tetras.at(ti).vertices[vi]);
  }

  Vertex &vertexOfTetra(int ti, int vi) {
    return vertices.at(tetras.at(ti).vertices[vi]);
  }

  std::vector<CavityTetrahedron> tetras;
  std::vector<Face> faces;
  std::vector<Vertex> vertices;
};

class PolyMesh {
 public:
  std::vector<Polyhedron> cells;
  std::vector<Tetrahedron> tetras;
  std::vector<Face> faces;
  std::vector<Vertex> vertices;
};

// MATH
bool isOutside(const Vertex &p0, const Vertex &p1, const Vertex &p2,
               const Vertex &v);

class Sphere {
 public:
  float radius{};
  Vertex center{};

  Sphere() = default;
  Sphere(const Vertex &v0, const Vertex &v1, const Vertex &v2,
         const Vertex &v3);
  [[nodiscard]] bool isInside(const Vertex &v) const;
};

// ALGORITHM
class Algorithm {
 public:
  virtual ~Algorithm() = default;
  virtual PolyMesh operator()(const Mesh &mesh) = 0;
};

class CavityAlgorithm : public Algorithm {
 public:
  class Criteria {
   public:
    virtual ~Criteria() = default;
    virtual void bind(CavityMesh *mesh) = 0;
    virtual void unbind() = 0;
    [[nodiscard]] virtual bool isNext(int current, int next) const = 0;
    [[nodiscard]] virtual std::vector<int> getSeeds() const = 0;
  };

  CavityAlgorithm(Criteria *criteria) : criteria(criteria) {};
  ~CavityAlgorithm() override = default;
  PolyMesh operator()(const Mesh &mesh) override;

 private:
  Criteria *criteria;
};

class SphereCriteria : public CavityAlgorithm::Criteria {
 public:
  void bind(CavityMesh *mesh) override;
  void unbind() override;
  bool isNext(int current, int next) const override;
  std::vector<int> getSeeds() const override;

 private:
  CavityMesh *mesh = nullptr;
  std::vector<Sphere> spheres;
};

// INPUT/OUTPUT
class Reader {
 public:
  virtual ~Reader() = default;
  virtual Mesh operator()(const std::string &filename) = 0;
};

class TetgenReader : public Reader {
 public:
  Mesh operator()(const std::string &filename) override;
  ~TetgenReader() override = default;
};

class Writer {
 public:
  virtual ~Writer() = default;
  virtual void operator()(const std::string &filename,
                          const PolyMesh &mesh) = 0;
};

class VisFWriter : public Writer {
 public:
  void operator()(const std::string &filename, const PolyMesh &mesh) override;
  ~VisFWriter() override = default;
};

};  // namespace gpolylla

#endif  // GPOLYLLA_H
