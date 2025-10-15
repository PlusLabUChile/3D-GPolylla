#ifndef POLYLLA_H
#define POLYLLA_H
#include <Eigen/Dense>
#include <array>
#include <functional>
#include <string>
#include <vector>

namespace Polylla
{

class PolyMesh;
class Mesh;

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
    float area(const Mesh &mesh) const;
};

struct Tetrahedron
{
    std::array<int, 4> vertices = {-1, -1, -1, -1};
    std::array<int, 4> faces = {-1, -1, -1, -1}; // Faces of the tetrahedron
    int polyhedron = -1;
    Tetrahedron();
    Tetrahedron(int v0, int v1, int v2, int v3);
    Tetrahedron(const std::array<int, 4> &verts);
    Tetrahedron(const std::array<int, 4> &verts, const std::array<int, 4> &faces);
    // Hash operator
    std::size_t hash() const;
    // Equality operator
    bool operator==(const Tetrahedron &other) const;
    float volume(const Mesh &mesh) const;
    float area(const Mesh &mesh) const;
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
    float area(const PolyMesh &mesh) const;
    float volume(const PolyMesh &mesh) const;
};

class Mesh
{
  public:
    std::vector<Vertex> vertices;
    std::vector<Face> faces;
    std::vector<Tetrahedron> tetras;
};

class PolyMesh : public Mesh
{
  public:
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
    std::string nodeFile;
    std::string eleFile;
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
    std::string outputFile;
    void writeMesh(PolyMesh mesh) override;
};

class Algorithm
{
  public:
    virtual ~Algorithm() = default;
    virtual PolyMesh operator()(const Mesh &mesh) = 0;
};

class CavityAlgorithm : public Algorithm
{
  public:
    PolyMesh operator()(const Mesh &mesh) override;

    struct Cavity
    {
        double radius;
        Vertex center;
        int tetra;

        bool isInside(const Vertex &p) const;
    };

    const std::vector<Cavity> &cavities() const
    {
        return cavities_;
    };
    const std::vector<int> &owners() const
    {
        return owners_;
    };
    const std::vector<int> &seeds() const
    {
        return seeds_;
    };

  private:
    std::vector<Cavity> cavities_;
    std::vector<int> owners_;
    std::vector<int> seeds_;

    // struct Information
    // {
    //     struct
    //     {
    //         std::vector<double> radius;
    //         std::vector<Vertex> centers;
    //     } cavity;
    //
    //     struct
    //     {
    //         std::vector<int> seeds;
    //         std::vector<float> volumes;
    //         std::vector<float> areas;
    //         std::vector<float> hullVolumes;
    //         std::vector<float> hullAreas;
    //     } poly;
    // };
    //
    // Information info;
};
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
