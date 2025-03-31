#ifndef _GPOLYLLA_ALGO_H_
#define _GPOLYLLA_ALGO_H_

#include <gpolylla/mesh.h>
#include <gpolylla/utils.h>

#include <Eigen/Dense>
#include <set>

namespace gpolylla {
class Algorithm {
 public:
  virtual PolyMesh operator()(const TetraMesh &m) = 0;
  virtual ~Algorithm() = default;
};

struct AlgorithmStats {};

class FaceAlgorithm : public Algorithm {
 public:
  PolyMesh operator()(const TetraMesh &m);

  class Criteria {
   public:
    double value(int fi, const FaceAlgorithm &algo);
  };

 private:
  void depthFirstSearch(int tetra, std::vector<int> *faces,
                        std::set<int> *points);

  void calculateFittest(const std::vector<Face> &faces);

  std::vector<int> getSeeds();

  Criteria c;
  FaceTetraMesh mesh;
  std::vector<bool> visited;
  std::vector<int> fittests;
};

class CavityAlgorithm : public Algorithm {
 public:
  class Criteria {
   public:
    double value(int ti, const CavityAlgorithm &algo);
  };

  CavityAlgorithm() = default;

  CavityAlgorithm(Criteria c);

  PolyMesh operator()(const TetraMesh &m);

  AlgorithmStats stats() const;

 private:
  void depthFirstSearch(int tetra, std::vector<int> *faces,
                        std::set<int> *points, int seed);

  void calculateFittest();

  std::vector<int> getSeeds();

  Criteria c;
  CavityTetraMesh mesh;
  std::vector<bool> visited;
  std::vector<Sphere> fittests;
  PolyMesh ans;
};

}  // namespace gpolylla

#endif  // _GPOLYLLA_ALGO_H_
