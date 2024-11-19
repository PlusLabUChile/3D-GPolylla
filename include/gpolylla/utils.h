#ifndef _GPOLYLLA_UTILS_H_
#define _GPOLYLLA_UTILS_H_

#include <Eigen/Dense>

namespace gpolylla {

struct Sphere {
  Eigen::Vector3d center;
  double radius;

  bool isIn(const Eigen::Vector3d&) const;
};

inline double area(const Eigen::Vector3d& p0, const Eigen::Vector3d& p1,
                   const Eigen::Vector3d& p2) {
  return (p1 - p0).cross(p2 - p0).norm() * 0.5;
}

Sphere circumsphere(const Eigen::Vector3d& p0, const Eigen::Vector3d& p1,
                    const Eigen::Vector3d& p2, const Eigen::Vector3d& p3);

}  // namespace gpolylla

#endif  // _GPOLYLLA_UTILS_H_
