#include <gpolylla/algo.h>
#include <gpolylla/mesh.h>
#include <gpolylla/utils.h>

#include <map>
#include <set>

#include "../extern/polyhedron_kernel/polyhedron_kernel.h"
#include "Eigen/Core"

namespace gpolylla {
using namespace Eigen;

constexpr double TOLERANCE = 0.00000001;

// src: https://mathworld.wolfram.com/Circumsphere.html
Sphere circumsphere(const Eigen::Vector3d &p0, const Eigen::Vector3d &p1,
                    const Eigen::Vector3d &p2, const Eigen::Vector3d &p3) {
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

  Sphere sphere;
  sphere.center = Vector3d(Dx / (2 * a), Dy / (2 * a), Dz / (2 * a));
  sphere.radius = (sphere.center - p0).norm();
  return sphere;
}

bool Sphere::isIn(const Vector3d &point) const {
  return (center - point).norm() < radius + TOLERANCE;
}

void calculatePolyhedronKernel(const PolyMesh &mesh, AlgorithmStats *stats) {
  using namespace cinolib;
  using std::map;
  using std::set;
  using std::vector;

  for (const auto &cell : mesh.cells) {
    set<int> uniqueVerts;
    vector<vec3d> verts;
    vector<vector<uint>> polys;

    for (int fi : cell.faces) {
      for (int vi : mesh.faces[fi].vertices) {
        uniqueVerts.insert(vi);
      }
    }

    map<int, int> vertMap;
    for (int vi : uniqueVerts) {
      auto v = mesh.vertices[vi];
      verts.emplace_back(v.x, v.y, v.z);
      vertMap[vi] = verts.size() - 1;
    }

    for (int fi : cell.faces) {
      vector<uint> poly;
      for (int vi : mesh.faces[fi].vertices) {
        poly.push_back(vertMap[vi]);
      }
      polys.push_back(poly);
    }

    Polygonmesh<> m(verts, polys);

    PolyhedronKernel K;
    K.initialize(m.vector_verts());
    K.compute(m.vector_verts(), m.vector_polys(), m.vector_poly_normals());

    Polygonmesh<> kernel(K.kernel_verts, K.kernel_faces);
    std::cout << "Kernel: " << kernel.num_verts() << " verts, "
              << kernel.num_polys()
              << " faces, volume: " << kernel.mesh_volume() << std::endl;
  }
}

}  // namespace gpolylla
