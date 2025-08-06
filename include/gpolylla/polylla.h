#ifndef POLYLLA_H
#define POLYLLA_H
#include <Eigen/Dense>
#include <array>
#include <functional>
#include <string>
#include <vector>

namespace Polylla
{

struct Vertex : Eigen::Vector3f
{
    using Eigen::Vector3f::Vector3f;
};

struct Face
{
    std::array<int, 3> vertices = {-1, -1, -1};
    std::array<int, 2> tetras = {-1, -1}; // Neighbours
    Face();
    Face(int v0, int v1, int v2);
    Face(const std::array<int, 3> &verts);
    Face(const std::array<int, 3> &verts, const std::array<int, 2> &tets);
    // Hash operator
    std::size_t hash() const;
    // Equality operator
    bool operator==(const Face &other) const;
};

struct Tetrahedron
{
    std::array<int, 4> vertices = {-1, -1, -1, -1};
    std::array<int, 4> faces = {-1, -1, -1, -1}; // Faces of the tetrahedron
    Tetrahedron();
    Tetrahedron(int v0, int v1, int v2, int v3);
    Tetrahedron(const std::array<int, 4> &verts);
    Tetrahedron(const std::array<int, 4> &verts, const std::array<int, 4> &faces);
    // Hash operator
    std::size_t hash() const;
    // Equality operator
    bool operator==(const Tetrahedron &other) const;
};

struct Polyhedron
{
    std::vector<int> vertices;
    std::vector<int> faces;
    std::vector<int> cells;
    Polyhedron();
    Polyhedron(const std::vector<int> &verts);
    Polyhedron(const std::vector<int> &verts, const std::vector<int> &faces, const std::vector<int> &cells);
    // Hash operator
    std::size_t hash() const;
    // Equality operator
    bool operator==(const Polyhedron &other) const;
};

class Mesh
{
  public:
    std::vector<Vertex> vertices;
    std::vector<Face> faces;
    std::vector<Tetrahedron> cells;
};

class PolyMesh
{
  public:
    std::vector<Vertex> vertices;
    std::vector<Face> faces;
    std::vector<Tetrahedron> tetras;
    std::vector<Polyhedron> cells;
};

class Reader
{
  public:
    virtual ~Reader() = default;
    virtual Mesh readMesh() = 0;
};

class TetgenReader : public Reader
{
  public:
    std::string meshName;
    Mesh readMesh() override;
};

class Writer
{
  public:
    virtual void writeMesh(PolyMesh mesh) = 0;
    virtual ~Writer() = default;
};

class VisFWriter : public Writer
{
  public:
    std::string meshName;
    void writeMesh(PolyMesh mesh) override;
};

class Algorithm
{
  public:
    virtual ~Algorithm() = default;
    virtual PolyMesh operator()(const Mesh &mesh) = 0;
};

// class CavityPolylla : public Polylla {
//  public:
//   PolyMesh operator()(const Mesh& mesh) override;
// };
} // namespace Polylla

// Hash function specializations for std::unordered_set and std::unordered_map
namespace std
{
template <> struct hash<Polylla::Face>
{
    std::size_t operator()(const Polylla::Face &f) const noexcept
    {
        return f.hash();
    }
};

template <> struct hash<Polylla::Tetrahedron>
{
    std::size_t operator()(const Polylla::Tetrahedron &t) const noexcept
    {
        return t.hash();
    }
};

template <> struct hash<Polylla::Polyhedron>
{
    std::size_t operator()(const Polylla::Polyhedron &p) const noexcept
    {
        return p.hash();
    }
};
} // namespace std

#endif // POLYLLA_H
