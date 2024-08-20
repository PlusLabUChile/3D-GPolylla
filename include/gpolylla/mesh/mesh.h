#ifndef _GPOLYLLA_MESH_H
#define _GPOLYLLA_MESH_H
#include <vector>
#include <gpolylla/mesh/face.h>
#include <gpolylla/mesh/edge.h>
#include <gpolylla/mesh/face.h>
#include <gpolylla/mesh/tetra.h>

namespace gpolylla {
class Mesh {

};

class TetraMesh: public Mesh {
 public:
  std::vector<Vertex> vertices;
  std::vector<Face> faces;
  std::vector<Tetrahedron> tetras;
  std::vector<Edge> edges;

 // public:
 //  TetraMesh(const std::string& node_filename, const std::string& face_filename,
 //       const std::string& ele_filename);
 //
 //  friend std::ostream& operator<<(std::ostream& out, const Mesh& m);
};

class PolyMesh: public Mesh {

};

}  // namespace gpolylla
#endif  // _GPOLYLLA_MESH_H
