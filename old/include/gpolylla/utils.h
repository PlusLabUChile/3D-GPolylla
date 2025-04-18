#ifndef _GPOLYLLA_UTILS_H_
#define _GPOLYLLA_UTILS_H_

#include <Eigen/Dense>

namespace gpolylla {

struct Sphere {
  Eigen::Vector3f center;
  double radius;

  bool isIn(const Eigen::Vector3f &) const;
};

inline Eigen::Vector3f normal(const Eigen::Vector3f &p0,
                              const Eigen::Vector3f &p1,
                              const Eigen::Vector3f &p2) {
  Eigen::Vector3f a = p1 - p0;
  Eigen::Vector3f b = p2 - p0;
  return a.cross(b);
}

inline double area(const Eigen::Vector3f &p0, const Eigen::Vector3f &p1,
                   const Eigen::Vector3f &p2) {
  return normal(p0, p1, p2).norm() * 0.5;
}

Sphere circumsphere(const Eigen::Vector3f &p0, const Eigen::Vector3f &p1,
                    const Eigen::Vector3f &p2, const Eigen::Vector3f &p3);

class PolyMesh;
class AlgorithmStats;
void calculatePolyhedronKernel(const PolyMesh &mesh, AlgorithmStats *stats);

}  // namespace gpolylla

#endif  // _GPOLYLLA_UTILS_H_
