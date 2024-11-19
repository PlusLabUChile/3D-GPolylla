#include <gpolylla/mesh.h>
#include <gtest/gtest.h>


namespace gpolylla {
class FaceTetraMeshTest : public ::testing::Test {
 protected:
  FaceTetraMeshTest() {
    tmesh.vertices = {{0, 0.0, 0.0, 0.0}, {1, 0.0, 0.0, 1.0},
                      {2, 1.0, 0.0, 1.0}, {3, 1.0, 0.0, 0.0},
                      {4, 0.0, 1.0, 0.0}, {5, 0.0, 1.0, 1.0},
                      {6, 1.0, 1.0, 1.0}, {7, 1.0, 1.0, 0.0}};

    tmesh.faces = {{0, 0, 1, 2},  {1, 2, 3, 0},  {2, 2, 3, 7},  {3, 7, 6, 2},
                   {4, 2, 1, 5},  {5, 5, 6, 2},  {6, 5, 6, 7},  {7, 7, 4, 5},
                   {8, 0, 4, 7},  {9, 7, 3, 0},  {10, 0, 1, 5}, {11, 5, 4, 0},
                   {12, 0, 5, 2}, {13, 0, 5, 7}, {14, 5, 2, 7}, {15, 0, 7, 2}};

    tmesh.tetras = {
        {0, 0, 1, 2, 5}, {1, 0, 2, 3, 7}, {2, 2, 5, 6, 7},
        {3, 5, 7, 0, 4}, {4, 5, 2, 0, 7},
    };
  };
  TetraMesh tmesh;
};


TEST_F(FaceTetraMeshTest, CreationTest) {
  FaceTetraMesh fmesh(tmesh);
  // std::cout << fmesh << std::endl;
  // std::cout << tmesh << std::endl;
  // ASSERT_TRUE(false);
}
}  // namespace gpolylla
