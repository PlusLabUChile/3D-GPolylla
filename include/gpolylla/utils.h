#ifndef _GPOLYLLA_UTILS_H_
#define _GPOLYLLA_UTILS_H_

#include <gpolylla/mesh.h>

#include <Eigen/Dense>

#include "Eigen/Core"

namespace gpolylla {

inline double area(Eigen::Vector3d p0, Eigen::Vector3d p1, Eigen::Vector3d p2) {
  return (p1 - p0).cross(p2 - p0).norm() * 0.5;
}

Eigen::Vector3d circumsphere(Eigen::Vector3d p0, Eigen::Vector3d p1,
                             Eigen::Vector3d p2, Eigen::Vector3d p3);

}  // namespace gpolylla

#endif  // _GPOLYLLA_UTILS_H_
