#include <filesystem>
#include <fstream>
#include <gpolylla/partition.h>
#include <gtest/gtest.h>
#include <numeric>
#include <set>
using namespace Polylla;
namespace
{
Mesh load(const std::string &name)
{
    TetgenReader reader;
    reader.nodeFile = std::string(DATA_DIR) + name + ".node";
    reader.eleFile = std::string(DATA_DIR) + name + ".ele";
    return reader.readMesh();
}
} // namespace
TEST(PartitionTest, AllLegacyMeshesHaveExactConnectedPartitions)
{
    for (auto name : {"basic", "minimal", "3D_100", "socket", "1000points", "1000points07", "mage", "angel"})
    {
        SCOPED_TRACE(name);
        auto mesh = load(name);
        CavityAlgorithm algorithm;
        auto result = algorithm(mesh);
        EXPECT_NO_THROW(validatePartition(result));
        double source = 0, partition = 0;
        for (const auto &tetra : mesh.tetras)
            source += std::abs(tetra.volume(mesh));
        for (const auto &cell : result.cells)
            for (int ti : cell.cells)
                partition += std::abs(result.tetras[ti].volume(result));
        EXPECT_NEAR(source, partition, source * 1e-9);
        std::vector<int> membership;
        for (const auto &tetra : result.tetras)
            membership.push_back(tetra.polyhedron);
        auto rebuilt = fromMembership(mesh, membership);
        EXPECT_EQ(result.cells.size(), rebuilt.cells.size());
    }
}
TEST(PartitionTest, SeedOrdersAndScaleAreDeterministic)
{
    auto mesh = load("basic");
    for (auto order : {CavityAlgorithm::SeedOrder::Ascending, CavityAlgorithm::SeedOrder::Descending,
                       CavityAlgorithm::SeedOrder::Index, CavityAlgorithm::SeedOrder::Random})
    {
        CavityAlgorithm algorithm;
        algorithm.seedOrder = order;
        auto reference = algorithm(mesh);
        for (double scale : {0.001, 1.0, 1000.0})
        {
            auto scaled = mesh;
            for (auto &v : scaled.vertices)
                v = v * scale + Eigen::Vector3d(2, -3, 4);
            auto result = algorithm(scaled);
            EXPECT_NO_THROW(validatePartition(result));
            ASSERT_EQ(result.tetras.size(), reference.tetras.size());
            for (int i = 0; i < result.tetras.size(); ++i)
                EXPECT_EQ(result.tetras[i].polyhedron, reference.tetras[i].polyhedron);
        }
    }
}
TEST(PartitionTest, InvalidMembershipIsRejected)
{
    auto mesh = load("basic");
    EXPECT_THROW(fromMembership(mesh, {}), std::runtime_error);
    EXPECT_THROW(fromMembership(mesh, std::vector<int>(mesh.tetras.size(), -1)), std::runtime_error);
    auto result = CavityAlgorithm()(mesh);
    result.cells[0].faces.clear();
    EXPECT_THROW(validatePartition(result), std::runtime_error);
}
TEST(ReaderTest, OneBasedAttributesCommentsAndBlankLines)
{
    auto dir = std::filesystem::temp_directory_path() / "gpolylla-reader-test";
    std::filesystem::create_directories(dir);
    auto node = dir / "test.node", ele = dir / "test.ele";
    {
        std::ofstream f(node);
        f << "# header\n\n4 3 1 1\n1 0 0 0 2 1\n2 1 0 0 2 1\n3 0 1 0 2 1 # inline\n4 0 0 1 2 1\n";
    }
    {
        std::ofstream f(ele);
        f << "1 4 1\n1 1 2 3 4 9\n";
    }
    TetgenReader reader;
    reader.nodeFile = node.string();
    reader.eleFile = ele.string();
    auto mesh = reader.readMesh();
    EXPECT_EQ(mesh.vertices.size(), 4);
    EXPECT_EQ(mesh.tetras.size(), 1);
    EXPECT_DOUBLE_EQ(std::abs(mesh.tetras[0].volume(mesh)), 1.0 / 6.0);
    {
        std::ofstream f(ele);
        f << "2 4 0\n1 1 2 3 4\n";
    }
    EXPECT_THROW(reader.readMesh(), std::runtime_error);
    reader.nodeFile += ".missing";
    EXPECT_THROW(reader.readMesh(), std::runtime_error);
    std::filesystem::remove_all(dir);
}
TEST(ReaderTest, DegenerateCircumsphereIsRejected)
{
    Mesh mesh;
    mesh.vertices = {Vertex(0, 0, 0), Vertex(1, 0, 0), Vertex(0, 1, 0), Vertex(1, 1, 0)};
    mesh.tetras.emplace_back(0, 1, 2, 3);
    EXPECT_THROW(CavityAlgorithm()(mesh), std::runtime_error);
}

TEST(PartitionTest, KuhnCubeEqualRadiiAndSliver)
{
    Mesh mesh;
    for (int x = 0; x < 2; ++x)
        for (int y = 0; y < 2; ++y)
            for (int z = 0; z < 2; ++z)
                mesh.vertices.emplace_back(x, y, z);
    std::array<int, 3> axes = {0, 1, 2};
    do
    {
        int v = 0;
        std::array<int, 4> tetra;
        tetra[0] = 0;
        for (int j = 0; j < 3; ++j)
        {
            v |= 1 << (2 - axes[j]);
            tetra[j + 1] = v;
        }
        mesh.tetras.emplace_back(tetra);
    } while (std::next_permutation(axes.begin(), axes.end()));
    // Roundtrip through the reader builds canonical incidence.
    auto dir = std::filesystem::temp_directory_path() / "gpolylla-kuhn-test";
    std::filesystem::create_directories(dir);
    auto node = dir / "mesh.node", ele = dir / "mesh.ele";
    {
        std::ofstream f(node);
        f << "8 3 0 0\n";
        for (int i = 0; i < 8; ++i)
            f << i << ' ' << mesh.vertices[i].transpose() << '\n';
    }
    {
        std::ofstream f(ele);
        f << "6 4 0\n";
        for (int i = 0; i < 6; ++i)
        {
            f << i;
            for (int v : mesh.tetras[i].vertices)
                f << ' ' << v;
            f << '\n';
        }
    }
    TetgenReader reader;
    reader.nodeFile = node.string();
    reader.eleFile = ele.string();
    mesh = reader.readMesh();
    auto first = CavityAlgorithm()(mesh);
    EXPECT_NO_THROW(validatePartition(first));
    for (int repeat = 0; repeat < 5; ++repeat)
    {
        auto result = CavityAlgorithm()(mesh);
        for (int i = 0; i < 6; ++i)
            EXPECT_EQ(first.tetras[i].polyhedron, result.tetras[i].polyhedron);
    }
    {
        std::ofstream f(node);
        f << "4 3 0 0\n0 0 0 0\n1 1 0 0\n2 0 1 0\n3 0.2 0.2 1e-8\n";
    }
    {
        std::ofstream f(ele);
        f << "1 4 0\n0 0 1 2 3\n";
    }
    mesh = reader.readMesh();
    CavityAlgorithm algorithm;
    auto sliver = algorithm(mesh);
    EXPECT_TRUE(algorithm.cavities()[0].center.allFinite());
    EXPECT_TRUE(std::isfinite(algorithm.cavities()[0].radius));
    EXPECT_NO_THROW(validatePartition(sliver));
    std::filesystem::remove_all(dir);
}
