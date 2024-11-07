#include <gpolylla/utils.h>

#include "Eigen/Core"

namespace gpolylla {
using namespace Eigen;

// src: https://mathworld.wolfram.com/Circumsphere.html
Eigen::Vector3d circumsphere(Eigen::Vector3d p0, Eigen::Vector3d p1,
                             Eigen::Vector3d p2, Eigen::Vector3d p3) {
  using namespace Eigen;
  Matrix4d A, X, Y, Z;

  A << p0.x(), p0.y(), p0.z(), 1, p1.x(), p1.y(), p1.z(), 1, p2.x(), p2.y(),
      p2.z(), 1, p3.x(), p3.y(), p3.z(), 1;

  X << p0.squaredNorm(), p0.y(), p0.z(), 1, p1.squaredNorm(), p1.y(), p1.z(), 1,
      p2.squaredNorm(), p2.y(), p2.z(), 1, p3.squaredNorm(), p3.y(), p3.z(), 1;

  Y << p0.squaredNorm(), p0.x(), p0.z(), 1, p1.squaredNorm(), p1.x(), p1.z(), 1,
      p2.squaredNorm(), p2.x(), p2.z(), 1, p3.squaredNorm(), p3.x(), p3.z(), 1;

  Z << p0.squaredNorm(), p0.x(), p0.y(), 1, p1.squaredNorm(), p1.x(), p1.y(), 1,
      p2.squaredNorm(), p2.x(), p2.y(), 1, p3.squaredNorm(), p3.x(), p3.y(), 1;

  double a = A.determinant();
  double Dx = X.determinant();
  double Dy = -Y.determinant();
  double Dz = Z.determinant();

  return Vector3d(Dx / (2 * a), Dy / (2 * a), Dz / (2 * a));
}


}  // namespace gpolylla
