#include <gpolylla/mesh/edge.h>
#include <gtest/gtest.h>

#include <unordered_map>

namespace gpolylla {

class EdgeTest : public ::testing::Test {
 protected:
  EdgeTest() : v0(0), v1(1), v2(2), v3(3), e0(0), e1(1) {
    e0.vertices[0] = &v0;
    e0.vertices[1] = &v1;
    e1.vertices[0] = &v2;
    e1.vertices[1] = &v3;
  }

  Vertex v0, v1, v2, v3;
  Edge e0, e1;
};

TEST_F(EdgeTest, CreationTest) {
  ASSERT_EQ(*e0.vertices[0], v0);
  ASSERT_EQ(*e0.vertices[1], v1);
}

TEST_F(EdgeTest, OperatorTest) {
  Edge dummy(0);
  dummy.vertices[0] = &v0;
  dummy.vertices[1] = &v1;
  ASSERT_TRUE(e0 == dummy);
  ASSERT_FALSE(e0 == e1);
}

TEST_F(EdgeTest, CopyTest) {
  Edge dummy(e0);
  ASSERT_EQ(e0, dummy);
}

TEST_F(EdgeTest, HashTest) {
  std::unordered_map<Edge, int, Edge::Hash> map;
  map[e0] = 100;
  map[e1] = 200;
  ASSERT_EQ(map[e0], 100);
  ASSERT_EQ(map[e1], 200);

  Edge dummy(e0);
  ASSERT_EQ(map[e0], map[dummy]);
  ASSERT_NE(map[e0], map[e1]);
}
}  // namespace gpolylla
