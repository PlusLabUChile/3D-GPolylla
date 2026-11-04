#include "face_repair.h"
#include <chrono>
#include <fstream>
#include <gpolylla/partition.h>
#include <iomanip>
#include <iostream>
#include <map>
#include <numeric>
#include <random>
#include <set>
using namespace Polylla;
using Clock = std::chrono::steady_clock;
// Baseline without repair: connected components of non-frontier faces.
std::vector<int> faceMembership(const Mesh &mesh, bool incircle, bool repair = false)
{
    std::vector<int> best(mesh.tetras.size());
    std::vector<double> score(mesh.faces.size());
    for (int fi = 0; fi < mesh.faces.size(); ++fi)
    {
        const auto &f = mesh.faces[fi];
        const auto &a = mesh.vertices[f.vertices[0]], &b = mesh.vertices[f.vertices[1]],
                   &c = mesh.vertices[f.vertices[2]];
        double area2 = (b - a).cross(c - a).norm();
        score[fi] = incircle ? area2 / ((a - b).norm() + (b - c).norm() + (c - a).norm()) : area2;
    }
    double maximumScore = *std::max_element(score.begin(), score.end());
    for (double &value : score)
        value = std::round(value / maximumScore * 1e12);
    for (int ti = 0; ti < mesh.tetras.size(); ++ti)
    {
        best[ti] = mesh.tetras[ti].faces[0];
        for (int fi : mesh.tetras[ti].faces)
            if (score[fi] > score[best[ti]] || (score[fi] == score[best[ti]] && fi < best[ti]))
                best[ti] = fi;
    }
    if (repair)
        return repairedFaceMembership(mesh, best);
    std::vector<int> owner(mesh.tetras.size(), -1), stack;
    int pi = 0;
    for (int seed = 0; seed < mesh.tetras.size(); ++seed)
    {
        if (owner[seed] >= 0)
            continue;
        stack.push_back(seed);
        owner[seed] = pi;
        while (!stack.empty())
        {
            int ti = stack.back();
            stack.pop_back();
            for (int fi : mesh.tetras[ti].faces)
            {
                const auto &f = mesh.faces[fi];
                int next = f.tetras[0] == ti ? f.tetras[1] : f.tetras[0];
                if (next < 0 || owner[next] >= 0 || (best[ti] != fi && best[next] != fi))
                    continue;
                owner[next] = pi;
                stack.push_back(next);
            }
        }
        ++pi;
    }
    return owner;
}
std::vector<int> pairMembership(const Mesh &mesh)
{
    std::vector<int> order(mesh.tetras.size()), owner(mesh.tetras.size(), -1);
    std::iota(order.begin(), order.end(), 0);
    std::sort(order.begin(), order.end(), [&](int a, int b) {
        double x = std::abs(mesh.tetras[a].volume(mesh)), y = std::abs(mesh.tetras[b].volume(mesh));
        return x == y ? a < b : x > y;
    });
    int pi = 0, merged = 0, target = mesh.tetras.size() / 5;
    for (int ti : order)
    {
        if (owner[ti] >= 0)
            continue;
        int next = -1;
        double maximum = -1;
        if (merged < target)
            for (int fi : mesh.tetras[ti].faces)
            {
                const auto &f = mesh.faces[fi];
                int n = f.tetras[0] == ti ? f.tetras[1] : f.tetras[0];
                if (n >= 0 && owner[n] < 0 && f.area(mesh) > maximum)
                {
                    maximum = f.area(mesh);
                    next = n;
                }
            }
        owner[ti] = pi;
        if (next >= 0)
        {
            owner[next] = pi;
            ++merged;
        }
        ++pi;
    }
    return owner;
}
int main(int argc, char **argv)
{
    try
    {
        if (argc < 5)
            throw std::runtime_error("usage: GPolyllaExperiment node ele variant output-prefix [seed]");
        auto start = Clock::now();
        TetgenReader reader;
        reader.nodeFile = argv[1];
        reader.eleFile = argv[2];
        auto mesh = reader.readMesh();
        auto read = Clock::now();
        std::string variant = argv[3], prefix = argv[4];
        PolyMesh result;
        if (variant.starts_with("cavity-"))
        {
            CavityAlgorithm algorithm;
            auto order = variant.substr(7);
            if (order == "descending")
                algorithm.seedOrder = CavityAlgorithm::SeedOrder::Descending;
            else if (order == "index")
                algorithm.seedOrder = CavityAlgorithm::SeedOrder::Index;
            else if (order == "random")
                algorithm.seedOrder = CavityAlgorithm::SeedOrder::Random;
            else if (order != "ascending")
                throw std::runtime_error("Unknown seed order");
            if (argc > 5)
                algorithm.randomSeed = std::stoul(argv[5]);
            result = algorithm(mesh);
            validatePartition(result);
        }
        else if (variant.starts_with("face-"))
            result = fromMembership(mesh, faceMembership(mesh, variant.find("incircle") != std::string::npos,
                                                         variant.ends_with("-repair")));
        else if (variant == "membership")
        {
            if (argc < 6)
                throw std::runtime_error("Membership file required");
            std::ifstream input(argv[5]);
            if (!input)
                throw std::runtime_error("Cannot open membership file");
            std::vector<int> owner;
            int id;
            while (input >> id)
                owner.push_back(id);
            if (!input.eof())
                throw std::runtime_error("Invalid membership token");
            result = fromMembership(mesh, owner);
        }
        else if (variant == "pairs")
            result = fromMembership(mesh, pairMembership(mesh));
        else if (variant == "identity")
        {
            std::vector<int> m(mesh.tetras.size());
            std::iota(m.begin(), m.end(), 0);
            result = fromMembership(mesh, m);
        }
        else
            throw std::runtime_error("Unknown variant");
        auto algorithmEnd = Clock::now();
        validatePartition(result);
        auto validated = Clock::now();
        std::ofstream membership(prefix + ".membership");
        if (!membership)
            throw std::runtime_error("Cannot write membership");
        for (const auto &t : result.tetras)
            membership << t.polyhedron << '\n';
        membership.close();
        auto written = Clock::now();
        auto ms = [](auto a, auto b) { return std::chrono::duration<double, std::milli>(b - a).count(); };
        std::size_t nested = 0, capacity = 0, flat = 0;
        for (const auto &p : result.cells)
        {
            nested += sizeof(Polyhedron) + (p.vertices.size() + p.faces.size() + p.cells.size()) * sizeof(int);
            capacity +=
                sizeof(Polyhedron) + (p.vertices.capacity() + p.faces.capacity() + p.cells.capacity()) * sizeof(int);
            flat += (p.vertices.size() + p.faces.size() + p.cells.size()) * sizeof(int);
        }
        flat = 0;
        bool measureFlat = argc > 6 && std::string(argv[6]) == "--flat";
        auto flatStart = Clock::now();
        if (measureFlat)
        {
            std::array<std::vector<int>, 3> flatData;
            std::array<std::vector<std::size_t>, 3> offsets;
            std::array<std::size_t, 3> totals = {0, 0, 0};
            for (const auto &p : result.cells)
            {
                totals[0] += p.vertices.size();
                totals[1] += p.faces.size();
                totals[2] += p.cells.size();
            }
            for (int k = 0; k < 3; ++k)
            {
                flatData[k].reserve(totals[k]);
                offsets[k].reserve(result.cells.size() + 1);
                offsets[k].push_back(0);
            }
            for (const auto &p : result.cells)
            {
                const std::vector<int> *lists[] = {&p.vertices, &p.faces, &p.cells};
                for (int k = 0; k < 3; ++k)
                {
                    flatData[k].insert(flatData[k].end(), lists[k]->begin(), lists[k]->end());
                    offsets[k].push_back(flatData[k].size());
                }
            }
            flat = sizeof(flatData) + sizeof(offsets);
            for (int k = 0; k < 3; ++k)
                flat += flatData[k].capacity() * sizeof(int) + offsets[k].capacity() * sizeof(std::size_t);
        }
        auto flatEnd = Clock::now();
        nested += sizeof(result.cells);
        capacity += sizeof(result.cells) + (result.cells.capacity() - result.cells.size()) * sizeof(Polyhedron);
        std::ofstream timing(prefix + ".timing.csv");
        timing << "vertices,tetrahedra,cells,read_ms,algorithm_ms,validation_ms,write_ms,total_ms,nested_size_bytes,"
                  "nested_capacity_bytes,flat_actual_bytes,flat_build_ms\n"
               << std::setprecision(17) << mesh.vertices.size() << ',' << mesh.tetras.size() << ','
               << result.cells.size() << ',' << ms(start, read) << ',' << ms(read, algorithmEnd) << ','
               << ms(algorithmEnd, validated) << ',' << ms(validated, written) << ',' << ms(start, written) << ','
               << nested << ',' << capacity << ',' << flat << ',' << ms(flatStart, flatEnd) << '\n';
        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
