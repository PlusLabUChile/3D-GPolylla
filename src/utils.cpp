#include <cinolib/meshes/meshes.h>
#include <gpolylla/algo.h>
#include <gpolylla/mesh.h>
#include <gpolylla/utils.h>
#include <polyhedron_kernel.h>

#include "Eigen/Core"

namespace gpolylla {
using namespace Eigen;

constexpr double TOLERANCE = 0.00000001;

// src: https://mathworld.wolfram.com/Circumsphere.html
Sphere circumsphere(const Eigen::Vector3d& p0, const Eigen::Vector3d& p1,
                    const Eigen::Vector3d& p2, const Eigen::Vector3d& p3) {
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

bool Sphere::isIn(const Vector3d& point) const {
  return (center - point).norm() < radius + TOLERANCE;
}

void calculatePolyhedronKernel(const PolyMesh& mesh, AlgorithmStats* stats) {
  using namespace cinolib;
  using std::vector;

  vector<vec3d> verts;
  vector<vector<uint>> polys;

  for (const auto& v : mesh.vertices) {
    verts.emplace_back(v.x, v.y, v.z);
  }

  for (const auto& p : mesh.cells) {
    vector<uint> poly;
    for (int fi : p.faces) {
      for (int vi : mesh.faces[fi].vertices) {
        poly.push_back(vi);
      }
    }
    polys.push_back(poly);
  }

  Polygonmesh<> m(verts, polys);

  auto start = std::chrono::steady_clock::now();

  PolyhedronKernel K;
  K.initialize(m.vector_verts());
  K.compute(m.vector_verts(), m.vector_polys(), m.vector_poly_normals());

  auto time = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::steady_clock::now() - start);

  Polygonmesh<> kernel(K.kernel_verts, K.kernel_faces);
  std::cout << "Kernel: " << kernel.num_verts() << " verts, "
            << kernel.num_polys() << " faces" << std::endl
            << "Elapsed time: " << time.count() << " ms" << std::endl;

  //   input.erase(input.end() - 4, input.end());
  //   std::string output = input + "_kernel.off";
  //   kernel.save(output.c_str());
  //   std::cout << "Saved in: " << output << std::endl;
}

}  // namespace gpolylla
