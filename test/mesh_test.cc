#include <iostream>
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
  void TearDown() override {} };

TEST_F(MeshTest, TestMesh) {
  ASSERT_EQ(1, 2);
}

}  // namespace gpolylla
//

