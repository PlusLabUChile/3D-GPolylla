#ifndef _GPOLYLLA_IO_H_
#define _GPOLYLLA_IO_H_

#include <gpolylla/mesh.h>

#include <string>

namespace gpolylla {
class Reader {
 public:
  virtual ~Reader() = default;
  virtual TetraMesh read(const std::string& name) = 0;
};

class TetgenReader : public Reader {
 public:
  TetraMesh read(const std::string& name) override;

 private:
  int loadNode(const std::string& filename);
  int loadEle(const std::string& filename);
  int loadEdge(const std::string& filename);
  int loadFace(const std::string& filename);
  int buildFaces();
  int buildEdges();

  TetraMesh mesh;
};

class Writer {
 public:
  virtual ~Writer() = default;
  virtual void write(const PolyMesh& mesh, const std::string& name) = 0;
};

class VisFWriter : public Writer {
 public:
  void write(const PolyMesh& mesh, const std::string& name) override;
};

}  // namespace gpolylla
#endif  // _GPOLYLLA_IO_H_
