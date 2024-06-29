#include <gpolylla/mesh.h>

#include <iostream>
#include <memory>

#include "gtest/gtest.h"

namespace gpolylla {

class MeshTest : public ::testing::Test {
 protected:
  std::unique_ptr<Mesh> mesh;
  void SetUp() override {
    mesh = std::make_unique<Mesh>(DATA_DIR "test.node", DATA_DIR "test.face",
                                  DATA_DIR "test.ele");
  }
};

TEST_F(MeshTest, ReadingFilesTest) {
  std::cout << *mesh;
  ASSERT_EQ(mesh->nodes.size(), 8);
  auto& nds = mesh->nodes;
  ASSERT_EQ(nds[0], (Vertex{0, 0.f, 0.f, 0.f}));
  ASSERT_EQ(nds[1], (Vertex{1, 1.f, 0.f, 0.f}));
  ASSERT_EQ(nds[2], (Vertex{2, 1.f, 1.f, 0.f}));
  ASSERT_EQ(nds[3], (Vertex{3, 0.f, 1.f, 0.f}));
  ASSERT_EQ(nds[4], (Vertex{4, 0.f, 0.f, 1.f}));
  ASSERT_EQ(nds[5], (Vertex{5, 1.f, 0.f, 1.f}));
  ASSERT_EQ(nds[6], (Vertex{6, 1.f, 1.f, 1.f}));
  ASSERT_EQ(nds[7], (Vertex{7, 0.f, 1.f, 1.f}));
  ASSERT_EQ(mesh->faces.size(), 12);
  ASSERT_EQ(mesh->tetras.size(), 3);
  auto& trs = mesh->tetras;

  // std::cout << "Vertices\n";
  // for (auto &v : mesh->nodes) 
  //   std::cout << v.idx << ": " << v.x << " " << v.y << " " << v.z << "\n";
  //
  // std::cout << "Faces\n";
  // for (auto &f : mesh->faces) {
  //   std::cout << f.idx << ": ";
  //   for (auto &v: f.vertices) {
  //     std::cout << v << " ";
  //   }
  //   std::cout << "\n";
  // }
  //
  // std::cout << "Tetrahedron\n";
  // for (auto &t : mesh->tetras) {
  //   std::cout << t.idx << ": ";
  //   for (auto &v: t.vertices) {
  //     std::cout << v << " ";
  //   }
  //   std::cout << "\n";
  // }
}

}  // namespace gpolylla
