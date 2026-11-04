#include "utils.h"
#include <fstream>
#include <gpolylla/polylla.h>
#include <gtest/gtest.h>
#include <sstream>
#include <string>

using namespace Polylla;

class VisFWriterTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        // Set up the temp directory path
        tempDir = TEMP_DIR;
        testMeshName = "test_mesh";
    }

    void TearDown() override
    {
        // Clean up the test file
        std::string filePath = tempDir + "/" + testMeshName + ".visf";
        std::remove(filePath.c_str());
    }

    std::string tempDir;
    std::string testMeshName;
};

TEST_F(VisFWriterTest, WriteBasicPolyMesh)
{
    // Create a VisFWriter instance
    VisFWriter writer;
    writer.outputFile = tempDir + "/" + testMeshName + ".visf";

    // Write the mesh to file
    writer.writeMesh(BASIC_POLY_MESH);

    // Read the created file
    std::ifstream file(writer.outputFile);
    ASSERT_TRUE(file.is_open()) << "Failed to open created visf file: " << writer.outputFile;

    std::stringstream fileContent;
    fileContent << file.rdbuf();
    file.close();

    // Expected output based on BASIC_POLY_MESH and writer logic
    std::stringstream expectedOutput;

    // Format: 2 (ascii) 2 (polyhedral mesh)
    expectedOutput << "2 2" << std::endl;

    // Number of vertices (8) and vertex coordinates
    expectedOutput << "8" << std::endl;
    expectedOutput << "0 0 0" << std::endl; // Vertex 0
    expectedOutput << "0 0 1" << std::endl; // Vertex 1
    expectedOutput << "1 0 1" << std::endl; // Vertex 2
    expectedOutput << "1 0 0" << std::endl; // Vertex 3
    expectedOutput << "0 1 0" << std::endl; // Vertex 4
    expectedOutput << "0 1 1" << std::endl; // Vertex 5
    expectedOutput << "1 1 1" << std::endl; // Vertex 6
    expectedOutput << "1 1 0" << std::endl; // Vertex 7

    // The merged cube has 12 boundary triangles; its 4 internal faces are omitted.
    // Boundary faces are oriented and reindexed by the writer.
    expectedOutput << "12" << std::endl;
    expectedOutput << "3 0 1 2" << std::endl;
    expectedOutput << "3 5 1 0" << std::endl;
    expectedOutput << "3 2 1 5" << std::endl;
    expectedOutput << "3 2 3 0" << std::endl;
    expectedOutput << "3 0 3 7" << std::endl;
    expectedOutput << "3 7 3 2" << std::endl;
    expectedOutput << "3 5 6 2" << std::endl;
    expectedOutput << "3 2 6 7" << std::endl;
    expectedOutput << "3 7 6 5" << std::endl;
    expectedOutput << "3 0 4 5" << std::endl;
    expectedOutput << "3 7 4 0" << std::endl;
    expectedOutput << "3 5 4 7" << std::endl;

    // Neighbor relationship between polygons (0 for now)
    expectedOutput << "0" << std::endl;

    // Number of polyhedra (1) and polyhedron definition
    expectedOutput << "1" << std::endl;
    expectedOutput << "12 0 1 2 3 4 5 6 7 8 9 10 11" << std::endl; // 12 boundary face indices

    // Compare the content
    EXPECT_EQ(fileContent.str(), expectedOutput.str()) << "File content doesn't match expected output";
}

TEST_F(VisFWriterTest, FileCreation)
{
    // Test that the file is actually created
    VisFWriter writer;
    writer.outputFile = tempDir + "/" + testMeshName + ".visf";

    // Check file doesn't exist before writing
    std::ifstream checkFile(writer.outputFile);
    EXPECT_FALSE(checkFile.is_open()) << "File should not exist before writing";
    checkFile.close();

    // Write the mesh
    writer.writeMesh(BASIC_POLY_MESH);

    // Check file exists after writing
    std::ifstream createdFile(writer.outputFile);
    EXPECT_TRUE(createdFile.is_open()) << "File should exist after writing";
    createdFile.close();
}
