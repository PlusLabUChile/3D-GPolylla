#include <gpolylla/mesh/vertex.h>

#include <gtest/gtest.h>

#include <unordered_map>

namespace gpolylla {

class VertexTest : public ::testing::Test {
 protected:
  VertexTest(): v0(0), v1(1) {
    v0.x = -.5f;
    v0.y = 1.f;
    v0.z = 2.f;
  }

  Vertex v0, v1;
};

TEST_F(VertexTest, ConstructorTest) {
  ASSERT_EQ(v0.x, -.5f);
  ASSERT_EQ(v0.y, 1.f);
  ASSERT_EQ(v0.z, 2.f);
}


TEST_F(VertexTest, OperatorTest) {
  Vertex dummy(0);
  dummy.x = -.5f;
  dummy.y = 1.f;
  dummy.z = 2.f;
  ASSERT_TRUE(v0 == dummy);
  ASSERT_FALSE(v0 == v1);
}

TEST_F(VertexTest, CopyTest) {
  Vertex dummy(v0);
  ASSERT_EQ(v0, dummy);
}

TEST_F(VertexTest, HashTest) {
  std::unordered_map<Vertex, int, Vertex::Hash> map;
  map[v0] = 100;
  map[v1] = 200;
  ASSERT_EQ(map[v0], 100);
  ASSERT_EQ(map[v1], 200);

  Vertex dummy(v0);
  ASSERT_EQ(map[v0], map[dummy]);
  ASSERT_NE(map[v0], map[v1]);
}

}


