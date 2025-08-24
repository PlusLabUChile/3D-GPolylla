#include "utils.h"

using namespace Polylla;


class TetgenReaderTest : public ::testing::Test
{
  protected:
    TetgenReader reader;
};




TEST_F(TetgenReaderTest, BasicReadMesh)
{
    reader.meshName = DATA_DIR "basic";
    Mesh mesh = reader.readMesh();

    ASSERT_EQ(mesh.vertices.size(), BASIC_MESH.vertices.size());
    ASSERT_EQ(mesh.cells.size(), BASIC_MESH.cells.size());
    ASSERT_EQ(mesh.faces.size(), BASIC_MESH.faces.size());

    /*
    0    0.0  0.0  0.0
    1    0.0  0.0  1.0
    2    1.0  0.0  1.0
    3    1.0  0.0  0.0
    4    0.0  1.0  0.0
    5    0.0  1.0  1.0
    6    1.0  1.0  1.0
    7    1.0  1.0  0.0
    */
    // checkIn(mesh.vertices, BASIC_MESH.vertices[0]);
    // checkIn(mesh.vertices, BASIC_MESH.vertices[1]);
    // checkIn(mesh.vertices, BASIC_MESH.vertices[2]);
    // checkIn(mesh.vertices, BASIC_MESH.vertices[3]);
    // checkIn(mesh.vertices, BASIC_MESH.vertices[4]);
    // checkIn(mesh.vertices, BASIC_MESH.vertices[5]);
    // checkIn(mesh.vertices, BASIC_MESH.vertices[6]);
    // checkIn(mesh.vertices, BASIC_MESH.vertices[7]);
    checkSimilar(mesh.vertices, BASIC_MESH.vertices, "Vertices");

    /*
    0 0 1 2 5
    1 0 2 3 7
    2 2 5 6 7
    3 5 7 0 4
    4 5 2 0 7
    */
    // checkIn(mesh.cells, BASIC_MESH.cells[0]);
    // checkIn(mesh.cells, BASIC_MESH.cells[1]);
    // checkIn(mesh.cells, BASIC_MESH.cells[2]);
    // checkIn(mesh.cells, BASIC_MESH.cells[3]);
    // checkIn(mesh.cells, BASIC_MESH.cells[4]);
    checkSimilar(mesh.cells, BASIC_MESH.cells, "Cells");

    /*
      0 0 1 2
      1 0 1 5
      2 2 3 0
      3 0 5 2
      4 0 7 2
      5 7 3 0
      6 5 4 0
      7 0 4 7
      8 0 5 7
      9 2 1 5
     10 2 3 7
     11 5 6 2
     12 5 2 7
     13 7 6 2
     14 7 4 5
     15 5 6 7
    */
    // checkIn(mesh.faces, BASIC_MESH.faces[0]);
    // checkIn(mesh.faces, BASIC_MESH.faces[1]);
    // checkIn(mesh.faces, BASIC_MESH.faces[2]);
    // checkIn(mesh.faces, BASIC_MESH.faces[3]);
    // checkIn(mesh.faces, BASIC_MESH.faces[4]);
    // checkIn(mesh.faces, BASIC_MESH.faces[5]);
    // checkIn(mesh.faces, BASIC_MESH.faces[6]);
    // checkIn(mesh.faces, BASIC_MESH.faces[7]);
    // checkIn(mesh.faces, BASIC_MESH.faces[8]);
    // checkIn(mesh.faces, BASIC_MESH.faces[9]);
    // checkIn(mesh.faces, BASIC_MESH.faces[10]);
    // checkIn(mesh.faces, BASIC_MESH.faces[11]);
    // checkIn(mesh.faces, BASIC_MESH.faces[12]);
    // checkIn(mesh.faces, BASIC_MESH.faces[13]);
    // checkIn(mesh.faces, BASIC_MESH.faces[14]);
    // checkIn(mesh.faces, BASIC_MESH.faces[15]);
    checkSimilar(mesh.faces, BASIC_MESH.faces, "Faces");
}