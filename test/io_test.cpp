#include <gpolylla.h>
#include <gtest/gtest.h>

namespace gpolylla {

Mesh minimalMesh = {.vertices =
                        {
                            Vertex(0.0, 0.0, 0.0),
                            Vertex(0.0, 0.0, 1.0),
                            Vertex(1.0, 0.0, 1.0),
                            Vertex(1.0, 0.0, 0.0),
                            Vertex(0.0, 1.0, 0.0),
                            Vertex(0.0, 1.0, 1.0),
                            Vertex(1.0, 1.0, 1.0),
                            Vertex(1.0, 1.0, 0.0),

                        },
                    .faces =
                        {
                            Face({0, 1, 2}),
                            Face({2, 3, 0}),
                            Face({2, 3, 7}),
                            Face({7, 6, 2}),
                            Face({2, 1, 5}),
                            Face({5, 6, 2}),
                            Face({5, 6, 7}),
                            Face({7, 4, 5}),
                            Face({0, 4, 7}),
                            Face({7, 3, 0}),
                            Face({0, 1, 5}),
                            Face({5, 4, 0}),
                        },
                    .tetras = {
                        Tetrahedron({0, 1, 2, 5}),

                    }};

PolyMesh minimalPoly = {.vertices = minimalMesh.vertices,
                        .faces = minimalMesh.faces,
                        .tetras = minimalMesh.tetras,
                        .cells = {}};

class TetgenReaderTest : public ::testing::Test {
 protected:
  TetgenReader reader;
};

TEST_F(TetgenReaderTest, MinimalTest) {
  auto mesh = reader(DATA_DIR "minimal");

  ASSERT_EQ(mesh.vertices.size(), 8);
  ASSERT_EQ(mesh.tetras.size(), 5);
  ASSERT_EQ(mesh.faces.size(), 4 * mesh.tetras.size());
}

class VisFWriterTest : public ::testing::Test {
 protected:
  VisFWriter writer;
};

TEST_F(VisFWriterTest, MinimalTest) {
  PolyMesh m;

  m.vertices = {};

  m.faces = {

  };

  m.tetras = {};

  m.cells = {};
}

}  // namespace gpolylla
