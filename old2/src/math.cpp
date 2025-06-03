#include <Eigen/Dense>

#include "gpolylla.h"

namespace gpolylla {
using vec3 = Eigen::Vector3f;
using mat4 = Eigen::Matrix4f;
using number = float;

constexpr float TOLERANCE = 0.00000001;

vec3 toVector(const Vertex& v) { return {v.x, v.y, v.z}; }
Vertex toVertex(const vec3& v) { return {v.x(), v.y(), v.z()}; }

Sphere::Sphere(const Vertex& v0, const Vertex& v1, const Vertex& v2,
               const Vertex& v3) {
  auto vv0 = toVector(v0);
  auto vv1 = toVector(v1);
  auto vv2 = toVector(v2);
  auto vv3 = toVector(v3);

  mat4 A, X, Y, Z;

  A << vv0.x(), vv0.y(), vv0.z(), 1, vv1.x(), vv1.y(), vv1.z(), 1, vv2.x(),
      vv2.y(), vv2.z(), 1, vv3.x(), vv3.y(), vv3.z(), 1;

  X << vv0.squaredNorm(), vv0.y(), vv0.z(), 1, vv1.squaredNorm(), vv1.y(),
      vv1.z(), 1, vv2.squaredNorm(), vv2.y(), vv2.z(), 1, vv3.squaredNorm(),
      vv3.y(), vv3.z(), 1;

  Y << vv0.squaredNorm(), vv0.x(), vv0.z(), 1, vv1.squaredNorm(), vv1.x(),
      vv1.z(), 1, vv2.squaredNorm(), vv2.x(), vv2.z(), 1, vv3.squaredNorm(),
      vv3.x(), vv3.z(), 1;

  Z << vv0.squaredNorm(), vv0.x(), vv0.y(), 1, vv1.squaredNorm(), vv1.x(),
      vv1.y(), 1, vv2.squaredNorm(), vv2.x(), vv2.y(), 1, vv3.squaredNorm(),
      vv3.x(), vv3.y(), 1;

  const number a = A.determinant();
  const number Dx = X.determinant();
  const number Dy = -Y.determinant();
  const number Dz = Z.determinant();

  const vec3 c(Dx / (2 * a), Dy / (2 * a), Dz / (2 * a));
  center = toVertex(c);
  radius = (c - vv0).norm();
}

bool Sphere::isInside(const Vertex& v) const {
  const auto vv = toVector(v);
  const auto vc = toVector(center);
  return (vc - vv).norm() < radius + TOLERANCE;
}

vec3 normal(const Vertex& p0, const Vertex& p1, const Vertex& p2) {
  const auto v0 = toVector(p0);
  const auto v1 = toVector(p1);
  const auto v2 = toVector(p2);
  return (v1 - v0).cross(v2 - v0);
}

double area(const Vertex& p0, const Vertex& p1, const Vertex& p2) {
  return normal(p0, p1, p2).norm() * .5;
}

bool isOutside(const Vertex& p0, const Vertex& p1, const Vertex& p2,
               const Vertex& v) {
  const vec3 intern = toVector(v) - toVector(p0);
  return intern.dot(normal(p0, p1, p2)) <= 0;
}

}  // namespace gpolylla
