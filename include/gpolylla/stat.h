//
// Created by vigb9 on 06/10/2025.
//

#ifndef GPOLYLLA_STATS_H
#define GPOLYLLA_STATS_H
#include "polylla.h"
#include <vector>
#include <optional>

namespace Polylla
{
class Hull
{

    std::vector<Face> faces;
    std::vector<Vertex> vertices;

  public:
    Hull() = default;
    Hull(const Polyhedron &poly, const PolyMesh &mesh);
    float area() const;
    float volume() const;
};

class Kernel
{
    std::vector<Face> faces;
    std::vector<Vertex> vertices;

  public:
    Kernel() = default;
    Kernel(const Polyhedron &poly, const PolyMesh &mesh);
    float area() const;
    float volume() const;
    bool empty() const;
};

struct PolyStat
{
    float edgeRatio;
    float volumeRatio;
    float surfaceRatio;
    std::optional<Kernel> kernel;
    Hull hull;
};

// class MeshStat
// {
// public:
//   MeshStat(const PolyMesh& mesh);
//   std::vector<Kernel> kernels;
//   std::vector<Hull> hulls;
// };

std::vector<PolyStat> computeStats(const PolyMesh &mesh);

} // namespace Polylla

#endif // GPOLYLLA_STATS_H
