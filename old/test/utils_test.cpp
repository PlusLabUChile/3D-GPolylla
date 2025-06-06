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

        Sphere sphere = circumsphere(p0, p1, p2, p3);

        //  double radius = (center - p0).norm();

        ASSERT_EQ((sphere.center - p0).norm(), sphere.radius);
        ASSERT_EQ((sphere.center - p1).norm(), sphere.radius);
        ASSERT_EQ((sphere.center - p2).norm(), sphere.radius);
        ASSERT_EQ((sphere.center - p3).norm(), sphere.radius);
    }

    TEST(UtilsTest, CircumsphereRobustnessTest) {
        using namespace Eigen;
        // std::random_device dev;
        // std::mt19937 rng(dev);
        // std::uniform_real_distribution<double> dist();
        TetgenReader reader;
        std::string data[] = {"minimal", "socket", "1000points", "3D_100"};
        for (const auto &d: data) {
            std::string path(DATA_DIR);
            reader.setBasename(path.append(d));
            TetraMesh mesh = reader.build();
            for (auto &t: mesh.tetras) {
                auto sphere = circumsphere(
                        mesh.vertices[t.vertices[0]], mesh.vertices[t.vertices[1]],
                        mesh.vertices[t.vertices[2]], mesh.vertices[t.vertices[3]]);

                //      double radius = (center -
                //      Vector3d(mesh.vertices[t.vertices[0]])).norm();

                for (auto &vi: t.vertices) {
                    //        Vector3d point = mesh.vertices[vi];
                    //        ASSERT_NEAR(radius, (center - point).norm(), TOLERANCE);
                    ASSERT_TRUE(sphere.isIn(mesh.vertices[vi]));
                }
            }
        }
    }

    TEST(UtilsTest, BasicCrossTest) {
        using Eigen::Vector3d;
        Vector3d p0{0.f, 0.f, 0.f};
        Vector3d p1{1.f, 0.f, 0.f};
        Vector3d p2{0.f, 1.f, 0.f};

        Vector3d cross = normal(p0, p1, p2);

    }
}  // namespace gpolylla
