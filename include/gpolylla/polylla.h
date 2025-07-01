#ifndef POLYLLA_H
#define POLYLLA_H
#include <Eigen/Dense>
#include <array>
#include <string>
#include <vector>

namespace polylla {

class Vertex : public Eigen::Vector3f {
  using Eigen::Vector3f::Vector3f;
};

class Face {
 public:
  Face(int v1, int v2, int v3);
  Face(const std::array<int, 3>& verts);

  int operator[](int index) const;
  int& operator[](int index);
  int size() const;

  // Boolean operator
  bool operator==(const Face& other) const;

  // Iterators
  std::array<int, 3>::iterator begin();
  std::array<int, 3>::iterator end();
  std::array<int, 3>::const_iterator begin() const;
  std::array<int, 3>::const_iterator end() const;

 private:
  std::array<int, 3> vertices;
};

class Tetrahedron {
 public:
  Tetrahedron(int v1, int v2, int v3, int v4);
  Tetrahedron(const std::array<int, 4>& verts);

  int operator[](int index) const;
  int& operator[](int index);
  int size() const;

  // Boolean operator
  bool operator==(const Tetrahedron& other) const;

  // Iterators
  std::array<int, 4>::iterator begin();
  std::array<int, 4>::iterator end();
  std::array<int, 4>::const_iterator begin() const;
  std::array<int, 4>::const_iterator end() const;

 private:
  std::array<int, 4> vertices;
};

class Polyhedron {
 public:
  Polyhedron() = default;
  Polyhedron(const std::vector<int>& verts);

  int operator[](int index) const;
  int& operator[](int index);
  int size() const;
  bool empty() const;

  // Boolean operator
  bool operator==(const Polyhedron& other) const;

  // Iterators
  std::vector<int>::iterator begin();
  std::vector<int>::iterator end();
  std::vector<int>::const_iterator begin() const;
  std::vector<int>::const_iterator end() const;

  // Modifiers
  void add(int vertex);
  void clear();

 private:
  std::vector<int> vertices;
};

class Mesh {
 public:
  static Mesh fromFile(const std::string& node, const std::string& ele);
  std::vector<Vertex> vertices;
  std::vector<Face> faces;
  std::vector<Tetrahedron> cells;
};

class PolyMesh {
 public:
  void toFile(const std::string& visf);
  std::vector<Vertex> vertices;
  std::vector<Face> faces;
  std::vector<Polyhedron> cells;
};

class Polylla {
  virtual PolyMesh operator()(const Mesh& mesh) = 0;
  virtual ~Polylla() = 0;
};

// class CavityPolylla : public Polylla {
//  public:
//   PolyMesh operator()(const Mesh& mesh) override;
// };
}  // namespace polylla

#endif  // POLYLLA_H
