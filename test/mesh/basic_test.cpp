#include <gpolylla/mesh/basic.h>
#include <gtest/gtest.h>

#include <unordered_map>

namespace gpolylla {

class VertexTest : public ::testing::Test {
 protected:
  VertexTest() : v0(0), v1(1) {
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

class EdgeTest : public ::testing::Test {
 protected:
  EdgeTest() : /* v0(0), v1(1), v2(2), v3(3),  */e0(0), e1(1) {
    e0.vertices[0] = 0;
    e0.vertices[1] = 1;
    e1.vertices[0] = 2;
    e1.vertices[1] = 3;
  }

  // Vertex v0, v1, v2, v3;
  Edge e0, e1;
};

TEST_F(EdgeTest, CreationTest) {
  ASSERT_EQ(e0.vertices[0], 0);
  ASSERT_EQ(e0.vertices[1], 1);
}

TEST_F(EdgeTest, OperatorTest) {
  Edge dummy(0);
  dummy.vertices[0] = 0;
  dummy.vertices[1] = 1;
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

class FaceTest : public ::testing::Test {
 protected:
  FaceTest(): v0(0), v1(1), v2(2), v3(3), v4(4), v5(5), v6(6), f0(0), f1(1) {
    f0.vertices[0] = 0;
    f0.vertices[1] = 1;
    f0.vertices[2] = 2;
    f1.vertices[0] = 3;
    f1.vertices[1] = 4;
    f1.vertices[2] = 5;
  }

  Vertex v0, v1, v2, v3, v4, v5, v6;
  Face f0, f1;
};

TEST_F(FaceTest, CreationTest) {
  ASSERT_EQ(f0.vertices[0], 0);
  ASSERT_EQ(f0.vertices[1], 1);
  ASSERT_EQ(f0.vertices[2], 2);
}


TEST_F(FaceTest, OperatorTest) {
  Face dummy(0);
  dummy.vertices[0] = 0;
  dummy.vertices[1] = 1;
  dummy.vertices[2] = 2;
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

  Face dummy(1000);
  dummy.vertices = f0.vertices;
  ASSERT_EQ(map[f0], map[dummy]);
  ASSERT_NE(map[f0], map[f1]);
}

class TetraTest : public ::testing::Test {
 protected:
  TetraTest() : v0(0), v1(1), v2(2), v3(3), v4(4), v5(5), v6(6), v7(7), t0(0), t1(1) {
    t0.vertices[0] = 0;
    t0.vertices[1] = 1;
    t0.vertices[2] = 2;
    t0.vertices[3] = 3;
    t1.vertices[0] = 4;
    t1.vertices[1] = 5;
    t1.vertices[2] = 6;
    t1.vertices[3] = 7;
  }

  Vertex v0, v1, v2, v3, v4, v5, v6, v7;
  Tetrahedron t0, t1;
};

TEST_F(TetraTest, ConstructorTest) {
  ASSERT_EQ(t0.vertices[0], 0);
  ASSERT_EQ(t0.vertices[1], 1);
  ASSERT_EQ(t0.vertices[2], 2);
  ASSERT_EQ(t0.vertices[3], 3);
}

TEST_F(TetraTest, OperatorTest) {
  Tetrahedron dummy(0);
  dummy.vertices[0] = 0;
  dummy.vertices[1] = 1;
  dummy.vertices[2] = 2;
  dummy.vertices[3] = 3;
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
