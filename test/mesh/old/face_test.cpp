#include <gpolylla/mesh/face.h>

#include <gtest/gtest.h>

#include <unordered_map>

namespace gpolylla {

class FaceTest : public ::testing::Test {
 protected:
  FaceTest(): v0(0), v1(1), v2(2), v3(3), v4(4), v5(5), v6(6), f0(0), f1(1) {
    f0.vertices[0] = &v0;
    f0.vertices[1] = &v1;
    f0.vertices[2] = &v2;
    f1.vertices[0] = &v3;
    f1.vertices[1] = &v4;
    f1.vertices[2] = &v5;
  }

  Vertex v0, v1, v2, v3, v4, v5, v6;
  Face f0, f1;
};

TEST_F(FaceTest, CreationTest) {
  ASSERT_EQ(*f0.vertices[0], v0);
  ASSERT_EQ(*f0.vertices[1], v1);
  ASSERT_EQ(*f0.vertices[2], v2);
}


TEST_F(FaceTest, OperatorTest) {
  Face dummy(0);
  dummy.vertices[0] = &v0;
  dummy.vertices[1] = &v1;
  dummy.vertices[2] = &v2;
  ASSERT_TRUE(f0 == dummy);
  ASSERT_FALSE(f0 == f1);
}

TEST_F(FaceTest, CopyTest) {
  Face dummy(f0);
  ASSERT_EQ(f0, dummy);
}

TEST_F(FaceTest, HashTest) {
  std::unordered_map<Face, int, Face::Hash> map;
  map[f0] = 100;
  map[f1] = 200;
  ASSERT_EQ(map[f0], 100);
  ASSERT_EQ(map[f1], 200);

  Face dummy(f0);
  ASSERT_EQ(map[f0], map[dummy]);
  ASSERT_NE(map[f0], map[f1]);
}
}


