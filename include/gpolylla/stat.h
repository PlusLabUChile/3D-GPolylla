//
// Created by vigb9 on 06/10/2025.
//

#ifndef GPOLYLLA_STATS_H
#define GPOLYLLA_STATS_H
#include "polylla.h"
namespace Polylla
{
class Hull
{

  std::vector<Face> faces;
  std::vector<Vertex> vertices;
public:
  Hull(const Polyhedron& poly, const PolyMesh& mesh);
  float area() const;
  float volume() const;

};

class Kernel
{
  std::vector<Face> faces;
  std::vector<Vertex> vertices;
public:
  Kernel(const Polyhedron& poly, const PolyMesh& mesh);
  float area() const;
  float volume() const;

};
}





#endif // GPOLYLLA_STATS_H
