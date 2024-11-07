#ifndef _GPOLYLLA_IO_H_
#define _GPOLYLLA_IO_H_

#include <string>
#include <gpolylla/mesh.h>

namespace gpolylla {
class TetgenReader {
 public:
  void setBasename(const std::string& basename);
  TetraMesh build();
//
 private:
  int loadNode(const std::string& filename);
  int loadEle(const std::string& filename);
  int loadEdge(const std::string& filename);
  int loadFace(const std::string& filename);
  int buildFaces();
  int buildEdges();
private:
  TetraMesh _mesh;
  std::string _basename;
};
}  // namespace gpolylla
#endif  // _GPOLYLLA_IO_H_
