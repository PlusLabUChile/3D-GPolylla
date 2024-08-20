#include <gpolylla/mesh/tetra.h>
#include <gtest/gtest.h>

#include <unordered_map>

namespace gpolylla {

class TetraTest : public ::testing::Test {
 protected:
  TetraTest() : v0(0), v1(1), v2(2), v3(3), v4(4), v5(5), v6(6), v7(7), t0(0), t1(1) {
    t0.vertices[0] = &v0;
    t0.vertices[1] = &v1;
    t0.vertices[2] = &v2;
    t0.vertices[3] = &v3;
    t1.vertices[0] = &v4;
    t1.vertices[1] = &v5;
    t1.vertices[2] = &v6;
    t1.vertices[3] = &v7;
  }

  Vertex v0, v1, v2, v3, v4, v5, v6, v7;
  Tetrahedron t0, t1;
};

TEST_F(TetraTest, ConstructorTest) {
  ASSERT_EQ(*t0.vertices[0], v0);
  ASSERT_EQ(*t0.vertices[1], v1);
  ASSERT_EQ(*t0.vertices[2], v2);
  ASSERT_EQ(*t0.vertices[3], v3);
}

TEST_F(TetraTest, OperatorTest) {
  Tetrahedron dummy(0);
  dummy.vertices[0] = &v0;
  dummy.vertices[1] = &v1;
  dummy.vertices[2] = &v2;
  dummy.vertices[3] = &v3;
  ASSERT_TRUE(t0 == dummy);
  ASSERT_FALSE(t0 == t1);
}

TEST_F(TetraTest, CopyTest) {
  Tetrahedron dummy(t0);
  ASSERT_EQ(t0, dummy);
}

TEST_F(TetraTest, HashTest) {
  std::unordered_map<Tetrahedron, int, Tetrahedron::Hash> map;
  map[t0] = 100;
  map[t1] = 200;
  ASSERT_EQ(map[t0], 100);
  ASSERT_EQ(map[t1], 200);

  Tetrahedron dummy(t0);
  ASSERT_EQ(map[t0], map[dummy]);
  ASSERT_NE(map[t0], map[t1]);
}
}  // namespace gpolylla
