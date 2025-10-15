//
// Created by vigb9 on 06/10/2025.
//

#ifndef GPOLYLLA_STATS_H
#define GPOLYLLA_STATS_H
#include "polylla.h"
#include <vector>

namespace Polylla
{
class Hull
{

  std::vector<Face> faces;
  std::vector<Vertex> vertices;
public:
    Hull() = default;
  Hull(const Polyhedron& poly, const PolyMesh& mesh);
  float area() const;
  float volume() const;

};

class Kernel
{
  std::vector<Face> faces;
  std::vector<Vertex> vertices;
public:
    Kernel() = default;
  Kernel(const Polyhedron& poly, const PolyMesh& mesh);
  float area() const;
  float volume() const;

};


struct PolyStat
{
   float edgeRatio;
   float volumeRatio;
   float surfaceRatio;
    Kernel kernel;
    Hull hull;
};


// class MeshStat
// {
// public:
//   MeshStat(const PolyMesh& mesh);
//   std::vector<Kernel> kernels;
//   std::vector<Hull> hulls;
// };

std::vector<PolyStat> computeStats(const PolyMesh& mesh);


}





#endif // GPOLYLLA_STATS_H
