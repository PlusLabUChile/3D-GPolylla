//
// Created by vigb9 on 01-06-2025.
//
#include <array>
#include <fstream>
#include <set>
#include <sstream>
#include <string>

#include "gpolylla/polylla.h"

using namespace polylla;
using namespace Eigen;
using namespace std;

Face::Face(int v1, int v2, int v3) : vertices{v1, v2, v3} {}
Face::Face(const std::array<int, 3>& verts) : vertices(verts) {}
int Face::operator[](int i) const { return vertices[i]; }
int& Face::operator[](int i) { return vertices[i]; }
int Face::size() const { return vertices.size(); }
std::array<int, 3>::iterator Face::begin() { return vertices.begin(); }
std::array<int, 3>::iterator Face::end() { return vertices.end(); }
std::array<int, 3>::const_iterator Face::begin() const {
  return vertices.begin();
}
std::array<int, 3>::const_iterator Face::end() const { return vertices.end(); }
bool Face::operator==(const Face& other) const {
  return vertices == other.vertices;
}

Tetrahedron::Tetrahedron(int v1, int v2, int v3, int v4)
    : vertices{v1, v2, v3, v4} {}
Tetrahedron::Tetrahedron(const std::array<int, 4>& verts) : vertices(verts) {}
int Tetrahedron::operator[](int i) const { return vertices[i]; }
int& Tetrahedron::operator[](int i) { return vertices[i]; }
int Tetrahedron::size() const { return vertices.size(); }
std::array<int, 4>::iterator Tetrahedron::begin() { return vertices.begin(); }
std::array<int, 4>::iterator Tetrahedron::end() { return vertices.end(); }
std::array<int, 4>::const_iterator Tetrahedron::begin() const {
  return vertices.begin();
}
std::array<int, 4>::const_iterator Tetrahedron::end() const {
  return vertices.end();
}
bool Tetrahedron::operator==(const Tetrahedron& other) const {
  return vertices == other.vertices;
}

Polyhedron::Polyhedron(const std::vector<int>& verts) : vertices(verts) {}
int Polyhedron::operator[](int i) const { return vertices[i]; }
int& Polyhedron::operator[](int i) { return vertices[i]; }
int Polyhedron::size() const { return vertices.size(); }
bool Polyhedron::empty() const { return vertices.empty(); }
std::vector<int>::iterator Polyhedron::begin() { return vertices.begin(); }
std::vector<int>::iterator Polyhedron::end() { return vertices.end(); }
std::vector<int>::const_iterator Polyhedron::begin() const {
  return vertices.begin();
}
std::vector<int>::const_iterator Polyhedron::end() const {
  return vertices.end();
}
void Polyhedron::add(int vertex) { vertices.push_back(vertex); }
void Polyhedron::clear() { vertices.clear(); }
bool Polyhedron::operator==(const Polyhedron& other) const {
  return vertices == other.vertices;
}

std::vector<Vertex> readNodeFile(const std::string& nodeFile) {
  std::vector<Vertex> vertices;

  ifstream nodeStream(nodeFile);
  if (!nodeStream.is_open()) {
    throw runtime_error("Cannot open node file: " + nodeFile);
  }

  string line;
  getline(nodeStream, line);
  istringstream headerStream(line);
  int numVertices;
  headerStream >> numVertices;
  vertices.reserve(numVertices);

  while (getline(nodeStream, line)) {
    istringstream vertexStream(line);
    string token;
    vertexStream >> token;
    if (token == "#") continue;  // Skip comments
    float x, y, z;
    vertexStream >> x >> y >> z;
    vertices.emplace_back(x, y, z);
  }
  nodeStream.close();

  return vertices;
}

std::vector<Tetrahedron> readElementFile(const std::string& eleFile) {
  std::vector<Tetrahedron> cells;

  ifstream eleStream(eleFile);
  if (!eleStream.is_open()) {
    throw runtime_error("Cannot open element file: " + eleFile);
  }

  string line;
  getline(eleStream, line);
  istringstream eleHeaderStream(line);
  int numTetrahedra;
  eleHeaderStream >> numTetrahedra;
  cells.reserve(numTetrahedra);

  while (getline(eleStream, line)) {
    istringstream tetStream(line);
    string token;
    tetStream >> token;
    if (token == "#") continue;  // Skip comments
    int v1, v2, v3, v4;
    tetStream >> v1 >> v2 >> v3 >> v4;
    cells.emplace_back(v1, v2, v3, v4);
  }
  eleStream.close();

  return cells;
}

std::vector<Face> generateFacesFromTetrahedra(
    const std::vector<Tetrahedron>& cells) {
  std::set<std::array<int, 3>> uniqueFaces;

  for (const auto& tet : cells) {
    // Each tetrahedron has 4 faces
    std::array<std::array<int, 3>, 4> tetFaces = {{{tet[0], tet[1], tet[2]},
                                                   {tet[0], tet[1], tet[3]},
                                                   {tet[0], tet[2], tet[3]},
                                                   {tet[1], tet[2], tet[3]}}};

    for (auto face : tetFaces) {
      // Sort vertices to ensure consistent face representation
      ranges::sort(face);
      uniqueFaces.insert(face);
    }
  }

  // Convert set to vector of Face objects
  std::vector<Face> faces;
  faces.reserve(uniqueFaces.size());
  for (const auto& face : uniqueFaces) {
    faces.emplace_back(face[0], face[1], face[2]);
  }

  return faces;
}

Mesh Mesh::fromFile(const std::string& nodeFile, const std::string& eleFile) {
  Mesh mesh;

  // Read vertices and tetrahedra using helper functions
  mesh.vertices = readNodeFile(nodeFile);
  mesh.cells = readElementFile(eleFile);

  // Generate faces from tetrahedra using helper function
  mesh.faces = generateFacesFromTetrahedra(mesh.cells);

  return mesh;
}
