#include <gpolylla/utils.h>
#include <gtest/gtest.h>

#include <random>

#include "Eigen/Core"
#include "gpolylla/io.h"

namespace gpolylla {

TEST(UtilsTest, AreaTest) {
  using namespace Eigen;

  Vector3d p0(0.0, 0.0, 0.0);
  Vector3d p1(0.0, 0.0, 1.0);
  Vector3d p2(1.0, 0.0, 0.0);

  // p1 - p0 = 0 0 1
  // p2 - p0 = 1 0 0
  // cross = 0 1 0
  // norm = 1

  ASSERT_EQ(area(p0, p1, p2), 0.5);
}

TEST(UtilsTest, CircumsphereTest) {
  using namespace Eigen;

  Vector3d p0(0.0, 0.0, 0.0);
  Vector3d p1(1.0, 0.0, 0.0);
  Vector3d p2(0.0, 1.0, 0.0);
  Vector3d p3(0.0, 0.0, 1.0);

  Vector3d center = circumsphere(p0, p1, p2, p3);

  double radius = (center - p0).norm();

  ASSERT_EQ((center - p0).norm(), radius);
  ASSERT_EQ((center - p1).norm(), radius);
  ASSERT_EQ((center - p2).norm(), radius);
  ASSERT_EQ((center - p3).norm(), radius);
}

TEST(UtilsTest, CircumsphereRobustnessTest) {
  using namespace Eigen;
  double tolerance = 0.00000001;
  // std::random_device dev;
  // std::mt19937 rng(dev);
  // std::uniform_real_distribution<double> dist();
  TetgenReader reader;
  std::string data[] = {"minimal", "socket.1", "1000points.1", "3D_100.1"};
  for (const auto& d : data) {
    std::string path(DATA_DIR);
    reader.setBasename(path.append(d));
    TetraMesh mesh = reader.build();
    for (auto& t : mesh.tetras) {
      auto center = circumsphere(
          mesh.vertices[t.vertices[0]], mesh.vertices[t.vertices[1]],
          mesh.vertices[t.vertices[2]], mesh.vertices[t.vertices[3]]);

      double radius = (center - Vector3d(mesh.vertices[t.vertices[0]])).norm();

      for (auto& vi : t.vertices) {
        Vector3d point = mesh.vertices[vi];
        ASSERT_NEAR(radius, (center - point).norm(), tolerance);
      }
    }
  }
}
}  // namespace gpolylla
