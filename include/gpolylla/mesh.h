#ifndef _GPOLYLLA_MESH_H
#define _GPOLYLLA_MESH_H
#include <string>
#include <vector>
#include <gpolylla/vertex.h>
#include <gpolylla/edge.h>
#include <gpolylla/face.h>
#include <gpolylla/tetra.h>

namespace gpolylla {
class Mesh {

};

class TetraMesh: public Mesh {
 public:
  std::vector<Vertex> nodes;
  std::vector<Face> faces;
  std::vector<Tetrahedron> tetras;
  std::vector<Edge> edges;

 public:
  TetraMesh(const std::string& node_filename, const std::string& face_filename,
       const std::string& ele_filename);

  friend std::ostream& operator<<(std::ostream& out, const Mesh& m);
};

}  // namespace gpolylla
#endif  // _GPOLYLLA_MESH_H
