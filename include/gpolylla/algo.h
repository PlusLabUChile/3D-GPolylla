#ifndef _GPOLYLLA_ALGO_H_
#define _GPOLYLLA_ALGO_H_
#include <gpolylla/mesh.h>
#include <gpolylla/utils.h>

#include <Eigen/Dense>

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

class CavityAlgorithm {
 public:
  PolyMesh operator()(const TetraMesh& m);

  class Criteria {
   public:
    Sphere value(int ti, const CavityTetraMesh& mesh);
  };

 private:
  void depthFirstSearch(int tetra, std::vector<int>* polyhedron);
  void calculateFittest();
  std::vector<int> getSeeds();
  Criteria c;
  CavityTetraMesh mesh;
  std::vector<bool> visited;
  CavityTetrahedron* current;
};

}  // namespace gpolylla

#endif  // _GPOLYLLA_ALGO_H_
