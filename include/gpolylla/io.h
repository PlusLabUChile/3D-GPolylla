#ifndef _GPOLYLLA_IO_H_
#define _GPOLYLLA_IO_H_

#include <gpolylla/mesh.h>

#include <string>

namespace gpolylla {
class TetgenReader {
 public:
  void setBasename(const std::string& basename);
  TetraMesh build();

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

class VisFWriter {
 public:
  void setBasename(const std::string& basename);
  void write(const PolyMesh& mesh);

 private:
  std::string basename;
};
}  // namespace gpolylla
#endif  // _GPOLYLLA_IO_H_
