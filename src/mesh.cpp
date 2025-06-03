//
// Created by vigb9 on 01-06-2025.
//
#include "polylla.h"

using namespace Polylla;
using namespace Eigen;

Face::Face(int v1, int v2, int v3) : vertices(v1, v2, v3) {}
int Face::operator[](int i) const { return vertices[i]; }
int Face::size() const { return vertices.size(); }

Tetrahedron::Tetrahedron(int v1, int v2, int v3, int v4)
    : vertices(v1, v2, v3, v4) {}
int Tetrahedron::operator[](int i) const { return vertices[i]; }
int Tetrahedron::size() const { return vertices.size(); }
