#include <gpolylla/io/reader.h>
#include <gtest/gtest.h>

namespace gpolylla {
class TetgenTest : public ::testing::Test {
 protected:
  TetgenReader reader;
};

TEST_F(TetgenTest, ReadMinimalTest) { 
  reader.set_basename(DATA_DIR "minimal");
  TetraMesh m = reader.build(); 
  // std::cout << m << std::endl;
  ASSERT_EQ(m.vertices.size(), 8);
  ASSERT_EQ(m.edges.size(), 18);
  ASSERT_EQ(m.faces.size(), 16);
  ASSERT_EQ(m.tetras.size(), 5);
}
}  // namespace gpolylla
