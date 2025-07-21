#include <gpolylla/polylla.h>
#include <gtest/gtest.h>

#include <fstream>

using namespace Polylla;

class MeshTest : public ::testing::Test {};

TEST_F(MeshTest, SimpleTetgenRead) {
  std::vector vertices = {
      Vertex(0.0, 0.0, 0.0), Vertex(0.0, 0.0, 1.0), Vertex(1.0, 0.0, 1.0),
      Vertex(1.0, 0.0, 0.0), Vertex(0.0, 1.0, 0.0), Vertex(0.0, 1.0, 1.0),
      Vertex(1.0, 1.0, 1.0), Vertex(1.0, 1.0, 0.0),
  };
  std::vector cells = {Tetrahedron(0, 1, 2, 5), Tetrahedron(1, 2, 3, 7),
                       Tetrahedron(2, 5, 6, 7), Tetrahedron(5, 4, 7, 3),
                       Tetrahedron(4, 5, 2, 7)};

  // std::vector faces = {
  //     Face(),
  // };
  //
  std::ofstream f;
  f.open("temp.node", std::ios::out);
  f << vertices.size() << " 3 0 0\n";
  for (size_t i = 0; i < vertices.size(); ++i) {
    f << i << " " << vertices[i][0] << " " << vertices[i][1] << " "
      << vertices[i][2] << "\n";
  }
  f.close();
  f.open("temp.ele", std::ios::out);
  f << cells.size() << " 4 0\n";
  for (size_t i = 0; i < cells.size(); ++i) {
    f << i << " " << cells[i][0] << " " << cells[i][1] << " " << cells[i][2]
      << " " << cells[i][3] << "\n";
  }
  f.close();
  Mesh m = Mesh::fromFile("temp.node", "temp.ele");

  EXPECT_EQ(m.vertices.size(), vertices.size());
  EXPECT_EQ(m.vertices, vertices);
  EXPECT_EQ(m.cells.size(), cells.size());
  EXPECT_EQ(m.cells, cells);
}