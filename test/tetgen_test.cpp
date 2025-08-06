#include <gpolylla/polylla.h>
#include <gtest/gtest.h>

using namespace Polylla;

template <typename T> void checkIn(const std::vector<T> &vec, const T &value)
{
    ASSERT_TRUE(std::find(vec.begin(), vec.end(), value) != vec.end()) << "Value not found in vector.";
}

class TetgenReaderTest : public ::testing::Test
{
  protected:
    TetgenReader reader;
};

TEST_F(TetgenReaderTest, BasicReadMesh)
{
    reader.meshName = DATA_DIR "basic";
    Mesh mesh = reader.readMesh();

    ASSERT_EQ(mesh.vertices.size(), 8);
    ASSERT_EQ(mesh.cells.size(), 5);
    ASSERT_EQ(mesh.faces.size(), 16);

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
    checkIn(mesh.vertices, Vertex(0, 0, 0));
    checkIn(mesh.vertices, Vertex(0, 0, 1));
    checkIn(mesh.vertices, Vertex(1, 0, 1));
    checkIn(mesh.vertices, Vertex(1, 0, 0));
    checkIn(mesh.vertices, Vertex(0, 1, 0));
    checkIn(mesh.vertices, Vertex(0, 1, 1));
    checkIn(mesh.vertices, Vertex(1, 1, 1));
    checkIn(mesh.vertices, Vertex(1, 1, 0));

    /*
    0 0 1 2 5
    1 0 2 3 7
    2 2 5 6 7
    3 5 7 0 4
    4 5 2 0 7
    */
    checkIn(mesh.cells, Tetrahedron({1, 0, 2, 5}, {0, 1, 3, 9}));
    checkIn(mesh.cells, Tetrahedron({0, 2, 3, 7}, {2, 5, 4, 10}));
    checkIn(mesh.cells, Tetrahedron({2, 5, 6, 7}, {11, 12, 13, 15}));
    checkIn(mesh.cells, Tetrahedron({5, 7, 0, 4}, {8, 6, 7, 14}));
    checkIn(mesh.cells, Tetrahedron({5, 2, 0, 7}, {3, 4, 8, 12}));

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
    checkIn(mesh.faces, Face({0, 1, 2}, {0, -1}));
    checkIn(mesh.faces, Face({0, 1, 5}, {0, -1}));
    checkIn(mesh.faces, Face({2, 3, 0}, {1, -1}));
    checkIn(mesh.faces, Face({0, 5, 2}, {0, 4}));
    checkIn(mesh.faces, Face({0, 7, 2}, {1, 4}));
    checkIn(mesh.faces, Face({7, 3, 0}, {1, -1}));
    checkIn(mesh.faces, Face({5, 4, 0}, {3, -1}));
    checkIn(mesh.faces, Face({0, 4, 7}, {3, -1}));
    checkIn(mesh.faces, Face({0, 5, 7}, {3, 4}));
    checkIn(mesh.faces, Face({2, 1, 5}, {0, -1}));
    checkIn(mesh.faces, Face({2, 3, 7}, {1, -1}));
    checkIn(mesh.faces, Face({5, 6, 2}, {2, -1}));
    checkIn(mesh.faces, Face({5, 2, 7}, {2, 4}));
    checkIn(mesh.faces, Face({7, 6, 2}, {2, -1}));
    checkIn(mesh.faces, Face({7, 4, 5}, {3, -1}));
    checkIn(mesh.faces, Face({5, 6, 7}, {2, -1}));
}