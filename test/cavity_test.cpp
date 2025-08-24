#include "utils.h"

using namespace Polylla;

class CavityTest : public ::testing::Test {
    protected:
        CavityAlgorithm algorithm;

};

TEST_F(CavityTest, CavityAlgorithmExecution) {
    // Run the cavity algorithm
    PolyMesh result = algorithm(BASIC_MESH);
    
    // Verify the result is not empty
    EXPECT_FALSE(result.vertices.empty());
    EXPECT_FALSE(result.faces.empty());
    EXPECT_FALSE(result.tetras.empty());
    EXPECT_FALSE(result.cells.empty());
    
    // The result should contain the original vertices
    EXPECT_EQ(result.vertices.size(), BASIC_MESH.vertices.size());
}

TEST_F(CavityTest, BasicPolyMeshCreation) {
    PolyMesh result = algorithm(BASIC_MESH);
    checkSimilar(result.vertices, BASIC_POLY_MESH.vertices, "Vertices");
    checkSimilar(result.faces, BASIC_POLY_MESH.faces, "Faces");
    checkSimilar(result.tetras, BASIC_POLY_MESH.tetras, "Tetras");
    checkSimilar(result.cells, BASIC_POLY_MESH.cells, "Cells");
}
