#include <gpolylla/algo/face.h>
#include <gpolylla/io/reader.h>
#include <gtest/gtest.h>
#include "gpolylla/mesh/basic.h"

namespace gpolylla {
class FaceAlgorithmTest: public ::testing::Test {
protected:
  FaceAlgorithmTest(): algo(), reader() {}
  FaceAlgorithm algo;
  TetgenReader reader;
};


TEST_F(FaceAlgorithmTest, BasicMesh) {
  reader.set_basename(DATA_DIR "minimal");
  TetraMesh mesh = reader.build();
  PolyMesh pmesh = algo(mesh);
  std::cout << mesh << std::endl;
  std::cout << pmesh << std::endl;
  ASSERT_TRUE(false);
}

}
