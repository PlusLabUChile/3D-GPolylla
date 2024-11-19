#ifndef _GPOLYLLA_ALGO_FACE_H_
#define _GPOLYLLA_ALGO_FACE_H_
#include <gpolylla/mesh/face.h>
#include "gpolylla/mesh/basic.h"

namespace gpolylla {
class FaceAlgorithm {
 public:
  PolyMesh operator()(const TetraMesh& m);

  class Criteria {
   public:
    double value(int fi, const FaceTetraMesh& mesh);
  };

 private:
  void depthFirstSearch(int tetra, std::vector<int>* polyhedron);
  void calculateFittest(const std::vector<Face>& faces);
  std::vector<int> getSeeds();
  FaceAlgorithm::Criteria c;
  FaceTetraMesh mesh;
  std::vector<bool> visited;
};
}  // namespace gpolylla

#endif  // _GPOLYLLA_ALGO_FACE_H_
