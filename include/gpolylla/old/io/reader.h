#ifndef _GPOLYLLA_IO_READER_H_
#define _GPOLYLLA_IO_READER_H_

#include <gpolylla/mesh/basic.h>

#include <string>
// #include <unordered_map>
// #include <vector>
//
namespace gpolylla {
// class Mesh;
//
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
//   bool apply_connectivity();
//
//   struct FaceKey {
//     Vertex *v0, *v1, *v2;
//     FaceKey(const Face& f);
//     FaceKey(Vertex* v0, Vertex* v1, Vertex* v2);
//
//     bool operator==(const FaceKey& o) const;
//
//     struct Hash {
//       std::size_t operator()(const FaceKey& f) const;
//     };
//   };
//
//   struct EdgeKey {
//     Vertex *vi, *vf;
//     EdgeKey(const Edge& e);
//     EdgeKey(Vertex* vi, Vertex* vf);
//     bool operator==(const EdgeKey& o) const;
//
//     struct Hash {
//       std::size_t operator()(const EdgeKey& e) const;
//     };
//   };
//
//   using EdgeToFaceMap =
//       std::unordered_map<EdgeKey, std::vector<int>, EdgeKey::Hash>;
//   using FaceToTetraMap =
//       std::unordered_map<FaceKey, std::vector<int>, FaceKey::Hash>;
//
//   TetraMesh _mesh;
//   FaceToTetraMap _fMap;
//   EdgeToFaceMap _eMap;
private:
  TetraMesh _mesh;
  std::string _basename;
};
}  // namespace gpolylla
#endif  // _GPOLYLLA_BUILDER_H_
