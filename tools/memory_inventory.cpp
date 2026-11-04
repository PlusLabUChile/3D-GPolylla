#include <chrono>
#include <fstream>
#include <gpolylla/polylla.h>
#include <iomanip>
#include <iostream>
using namespace Polylla;
template <typename T> std::size_t allocated(const std::vector<T> &values)
{
    return sizeof(values) + values.capacity() * sizeof(T);
}
std::size_t common(const Mesh &mesh)
{
    return allocated(mesh.vertices) + allocated(mesh.faces) + allocated(mesh.tetras);
}
int main(int argc, char **argv)
{
    try
    {
        if (argc != 4)
            throw std::runtime_error("usage: GPolyllaMemory node ele output.csv");
        TetgenReader reader;
        reader.nodeFile = argv[1];
        reader.eleFile = argv[2];
        auto mesh = reader.readMesh();
        CavityAlgorithm algorithm;
        auto start = std::chrono::steady_clock::now();
        auto result = algorithm(mesh);
        auto end = std::chrono::steady_clock::now();
        std::size_t nested = allocated(result.cells);
        for (const auto &cell : result.cells)
            nested +=
                allocated(cell.vertices) + allocated(cell.faces) + allocated(cell.cells) - 3 * sizeof(std::vector<int>);
        // Polyhedron's three vector descriptors are already counted by allocated(cells).
        std::size_t auxiliary =
            allocated(algorithm.cavities()) + allocated(algorithm.owners()) + allocated(algorithm.seeds());
        auxiliary +=
            sizeof(algorithm) - sizeof(algorithm.cavities()) - sizeof(algorithm.owners()) - sizeof(algorithm.seeds());
        std::ofstream out(argv[3]);
        if (!out)
            throw std::runtime_error("Cannot create inventory");
        out << "input_capacity_bytes,output_common_capacity_bytes,output_cell_capacity_bytes,algorithm_retained_"
               "capacity_bytes,retained_sum_bytes,algorithm_ms\n"
            << std::setprecision(17) << common(mesh) << ',' << common(result) << ',' << nested << ',' << auxiliary
            << ',' << common(mesh) + common(result) + nested + auxiliary << ','
            << std::chrono::duration<double, std::milli>(end - start).count() << '\n';
        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
