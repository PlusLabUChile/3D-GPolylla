#include <gpolylla/io.h>
#include <gtest/gtest.h>

#include "gpolylla/mesh.h"

namespace gpolylla {
class VisFTest : public ::testing::Test {
 protected:
  VisFWriter writer;
};

TEST_F(VisFTest, WriteMinimalTest) {
  PolyMesh m;

  m.vertices = {
      Vertex(0, 0.0, 0.0, 0.0), Vertex(1, 0.0, 0.0, 1.0),
      Vertex(2, 1.0, 0.0, 1.0), Vertex(3, 1.0, 0.0, 0.0),
      Vertex(4, 0.0, 1.0, 0.0), Vertex(5, 0.0, 1.0, 1.0),
      Vertex(6, 1.0, 1.0, 1.0), Vertex(7, 1.0, 1.0, 0.0),
  };

  m.faces = {
      Face(0, 0, 1, 2), Face(1, 2, 3, 0), Face(2, 2, 3, 7),  Face(3, 7, 6, 2),
      Face(4, 2, 1, 5), Face(5, 5, 6, 2), Face(6, 5, 6, 7),  Face(7, 7, 4, 5),
      Face(8, 0, 4, 7), Face(9, 7, 3, 0), Face(10, 0, 1, 5), Face(11, 5, 4, 0),
  };

  m.cells = {Polyhedron(0, {0, 1, 2, 3, 4, 5, 6, 7})};

  writer.setBasename(DATA_DIR "test");
  writer.write(m);
}
}  // namespace gpolylla
