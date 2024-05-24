#include <fstream>
#include <functional>
#include <iostream>
#include <unordered_map>
#define DATA_DIR std::string(_DATA_DIR)
#include "gpolylla/mesh.h"
#include "gtest/gtest.h"

namespace gpolylla {

class MeshTest : public ::testing::Test {
 protected:
  TetrahedronMesh mesh;

  void SetUp() override {
    mesh = TetrahedronMesh(DATA_DIR + "/3D_100.1.node",
                           DATA_DIR + "/3D_100.1.ele");
  }
  void TearDown() override {}
};

TEST_F(MeshTest, TestMesh) { ASSERT_EQ(1, 2); }

class FaceTest : public ::testing::Test {
 protected:
  Vertex v1, v2, v3;

  void SetUp() override {
    v1.x = 1.0f;
    v1.y = 2.0f;
    v1.z = 3.0f;
    v2.x = 4.0f;
    v2.y = 6.0f;
    v2.z = 7.0f;
    v3.x = 8.0f;
    v3.y = 9.0f;
    v3.z = 10.0f;
  }
};

TEST_F(FaceTest, FaceHash) {
  Face f1, f2;
  std::unordered_map<Face, int> map;

  f1.vertices.push_back(&v1);
  f1.vertices.push_back(&v2);
  f1.vertices.push_back(&v3);

  f2.vertices.push_back(&v2);
  f2.vertices.push_back(&v3);
  f2.vertices.push_back(&v1);

  map[f1] = 0;
  ASSERT_EQ(map[f2], 0);
}

}  // namespace gpolylla
//
