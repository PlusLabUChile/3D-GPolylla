//
// Created by vigb9 on 01-06-2025.
//
#include <filesystem>
#include <gpolylla/polylla.h>
#include <gpolylla/stat.h>
#include <iostream>
#include <polyhedron_kernel.h>
#include <string>
#include <vector>

using namespace Polylla;


struct Times
{
    float execution;
    float read;
    float write;
};


void createStats(const std::string& file, const std::vector<PolyStat>& stats, const Times& times, const PolyMesh& polyMesh, const Mesh& mesh)
{

    std::vector<bool> points(mesh.vertices.size());
    for (const auto& poly : polyMesh.cells)
    {
        for (const int vi : poly.vertices)
        {
            points[vi] = true;
        }
    }

    float conversionRatio = static_cast<float>(std::ranges::count(points, true)) / static_cast<float>(points.size());
    float totalTime = times.execution + times.read + times.write;
    float avgVolumeRatio = .0f;
    float minVolumeRatio = std::numeric_limits<float>::max();
    int idxMinVolumeRatio = -1;
    float maxVolumeRatio = std::numeric_limits<float>::min();
    int idxMaxVolumeRatio = -1;
    float avgSurfaceRatio = .0f;
    float minSurfaceRatio = std::numeric_limits<float>::max();
    int idxMinSurfaceRatio = -1;
    float maxSurfaceRatio = std::numeric_limits<float>::min();
    int idxMaxSurfaceRatio = -1;
    float avgEdgeRatio = .0f;
    float minEdgeRatio = std::numeric_limits<float>::max();
    int idxMinEdgeRatio = -1;
    float maxEdgeRatio = std::numeric_limits<float>::min();
    int idxMaxEdgeRatio = -1;

    int i = 0;
    for (const auto& stat : stats)
    {
        avgVolumeRatio += stat.volumeRatio;
        if (stat.volumeRatio < minVolumeRatio)
        {
            minVolumeRatio = stat.volumeRatio;
            idxMinVolumeRatio = i;
        }
        if (stat.volumeRatio > maxVolumeRatio)
        {
            maxVolumeRatio = stat.volumeRatio;
            idxMaxVolumeRatio = i;
        }

        avgSurfaceRatio += stat.surfaceRatio;
        if (stat.surfaceRatio < minSurfaceRatio)
        {
            minSurfaceRatio = stat.surfaceRatio;
            idxMinSurfaceRatio = i;
        }
        if (stat.surfaceRatio > maxSurfaceRatio)
        {
            maxSurfaceRatio = stat.surfaceRatio;
            idxMaxSurfaceRatio = i;
        }

        avgEdgeRatio += stat.edgeRatio;
        if (stat.edgeRatio < minEdgeRatio)
        {
            minEdgeRatio = stat.edgeRatio;
            idxMinEdgeRatio = i;
        }
        if (stat.edgeRatio > maxEdgeRatio)
        {
            maxEdgeRatio = stat.edgeRatio;
            idxMaxEdgeRatio = i;
        }

        i++;
    }

    avgVolumeRatio /= stats.size();
    avgSurfaceRatio /= stats.size();
    avgEdgeRatio /= stats.size();


    std::ofstream csv(file);

    if (!csv.is_open())
    {
        throw std::runtime_error("Unable to create file: " + file);
    }

    csv << "tetras,cells,points coverage,";
    csv << "execution time,read time,write time,total time,";
    csv << "volume ratio (min),volume ratio (max),volume ratio (avg),";
    csv << "idx volume ratio (min),idx volume ratio (max),";
    csv << "surface ratio (min),surface ratio (max),surface ratio (avg),";
    csv << "idx surface ratio (min),idx surface ratio (max),";
    csv << "edge ratio (min),edge ratio (max),edge ratio (avg),";
    csv << "idx edge ratio (min),idx edge ratio (max)";
    csv << std::endl;

    csv << mesh.tetras.size() << "," << polyMesh.cells.size() << "," << conversionRatio << ",";
    csv << times.execution << "," << times.read << "," << times.write << "," << totalTime << ",";
    csv << minVolumeRatio << "," << maxVolumeRatio << "," << avgVolumeRatio << ",";
    csv << idxMinVolumeRatio << "," << idxMaxVolumeRatio << ",";
    csv << minSurfaceRatio << "," << maxSurfaceRatio << "," << avgSurfaceRatio;
    csv << idxMinSurfaceRatio << "," << idxMaxSurfaceRatio << ",";
    csv << minSurfaceRatio << "," << maxSurfaceRatio << "," << avgEdgeRatio;
    csv << idxMinEdgeRatio << "," << idxMaxEdgeRatio;
    csv << std::endl;

    csv.close();
}

void createOFF(const std::string& file, const Polyhedron& poly, const PolyMesh& mesh)
{
    std::ofstream off(file);
    if (!off.is_open())
    {
        throw std::runtime_error("Unable to create file: " + file);
    }
    off << "OFF" << std::endl;
    off << poly.vertices.size() << " " << poly.faces.size() << " 0" << std::endl;

    std::unordered_map<int, int> vertexMap;
    int i = 0;
    for (int vi : poly.vertices)
    {
        off << mesh.vertices[vi].x() << " " << mesh.vertices[vi].y() << " " << mesh.vertices[vi].z() << "\n";
        vertexMap[vi] = i;
        i++;
    }

    for (int fi : poly.faces)
    {
        const auto& face = mesh.faces.at(fi);
        off << face.vertices.size();
        for (int vi: face.vertices)
        {
            off << " " << vertexMap[vi];
        }
        off << "\n";
    }

    off.close();
}

void generatePolyhedrons(const std::string& folder, const std::vector<PolyStat>& stats, const PolyMesh& polyMesh)
{
    try
    {
        std::filesystem::create_directories(folder);

        int i = 0;
        for (const auto& cell: polyMesh.cells)
        {
            createOFF(folder + "/cell_" + std::to_string(i) + ".off", cell, polyMesh);
            i++;
        }

        std::ofstream csv(folder + "/details.csv");

        if (!csv.is_open())
        {
            throw std::runtime_error("Unable to create file: " + folder + "/details.csv");
        }

        csv << "idx,edge ratio,volume ratio,surface ratio,has kernel,";
        csv << "poly volume,poly surface,";
        csv << "hull volume,hull surface,";
        csv << "kernel volume,kernel surface";
        csv << "\n";


        i = 0;
        for (const auto& stat : stats)
        {
            const auto& poly = polyMesh.cells.at(i);
            csv << i << ",";
            csv << stat.edgeRatio << "," << stat.volumeRatio << "," << stat.surfaceRatio << "," << stat.kernel.has_value() << ",";
            csv << poly.volume(polyMesh) << "," << poly.area(polyMesh) << ",";
            csv << stat.hull.volume() << "," << stat.hull.area() << ",";
            if (stat.kernel.has_value())
            {
                csv << stat.kernel->volume() << "," << stat.kernel->area() << "," ;
            }
            else
            {
                csv << 0 << "," << 0 << "," ;
            }
            csv << "\n";
            i++;
        }

        csv.close();
    }
    catch (const std::filesystem::filesystem_error& e)
    {
        throw std::runtime_error("Unable to create directory: " + folder + " (" + e.what() + ")");
    }

}

void displayHelp()
{
    std::cout << "Usage: ";
}

void displayUsage(const char *prog_name)
{
    std::cerr << "Usage: " << prog_name << " -n <node_file> -e <ele_file> -o <output_file>" << std::endl;
}

int main(int argc, char *argv[])
{
    std::string nodeFile;
    std::string eleFile;
    std::string outputFile;
    bool makeStats = false;
    bool detailStats = false;

    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];
        if (arg == "--help")
        {
            displayHelp();
            return 0;
        }
        if (arg == "-n" || arg == "--node")
        {
            if (i + 1 < argc)
            {
                nodeFile = argv[++i];
                continue;
            }

            std::cerr << "-n option requires one argument." << std::endl;
            displayUsage(argv[0]);
            return 1;
        }
        if (arg == "-e" || arg == "--ele")
        {
            if (i + 1 < argc)
            {
                eleFile = argv[++i];
                continue;
            }
            std::cerr << "-e option requires one argument." << std::endl;
            displayUsage(argv[0]);
            return 1;
        }
        if (arg == "-o" || arg == "--output")
        {
            if (i + 1 < argc)
            {
                outputFile = argv[++i];
                continue;
            }
            std::cerr << "-o option requires one argument." << std::endl;
            displayUsage(argv[0]);
            return 1;
        }

        if (arg == "--make-stats")
        {
            makeStats = true;
            continue;
        }

        if (arg == "--detail-stats")
        {
            detailStats = true;
            continue;
        }

        std::cerr << "Unknown option: " << arg << std::endl;
        displayUsage(argv[0]);
        return 1;
    }

    if (nodeFile.empty() || eleFile.empty() || outputFile.empty())
    {
        displayUsage(argv[0]);
        return 1;
    }

    Times times;


    TetgenReader reader;
    reader.nodeFile = nodeFile;
    reader.eleFile = eleFile;
    auto t0 = std::chrono::high_resolution_clock::now();
    Mesh mesh = reader.readMesh();
    auto t1 = std::chrono::high_resolution_clock::now();
    times.read = std::chrono::duration<float, std::milli>(t1 - t0).count();



    t0 = std::chrono::high_resolution_clock::now();
    PolyMesh polyMesh = CavityAlgorithm()(mesh);
    t1 = std::chrono::high_resolution_clock::now();
    times.execution = std::chrono::duration<float, std::milli>(t1 - t0).count();


    VisFWriter writer;
    writer.outputFile = outputFile;
    t0 = std::chrono::high_resolution_clock::now();
    writer.writeMesh(polyMesh);
    t1 = std::chrono::high_resolution_clock::now();
    times.write = std::chrono::duration<float, std::milli>(t1 - t0).count();

    if (makeStats)
    {
        std::vector<PolyStat> stats = computeStats(polyMesh);
        std::string basename = outputFile.substr(0, outputFile.find_last_of('.'));
        std::string statsFile = basename + ".csv";
        createStats(statsFile, stats, times, polyMesh, mesh);

        if (detailStats)
        {
            std::string folder = basename + "_detail";
            generatePolyhedrons(folder, stats, polyMesh);
        }

    }

    // auto stats = computeStats(polyMesh);
    // PolyhedronKernel k;
    // std::vector<cinolib::vec3d> kVertices;
    // std::vector<std::vector<uint>> kFaces;
    //
    // kVertices.emplace_back(0, 0, 0);
    // kVertices.emplace_back(1, 0, 0);
    // kVertices.emplace_back(1, 1, 0);
    // kVertices.emplace_back(0, 1, 0);
    // kVertices.emplace_back(0, 1, 1);
    // kVertices.emplace_back(1, 1, 1);
    // kVertices.emplace_back(1, 0, 1);
    // kVertices.emplace_back(0, 0, 1);
    //
    // // kFaces.push_back({3, 2, 1, 0});
    // kFaces.push_back({3,2,0});
    // kFaces.push_back({2,1,0});
    // // kFaces.push_back({1, 2, 5, 6});
    // kFaces.push_back({1, 2, 6});
    // kFaces.push_back({2, 5, 6});
    // // kFaces.push_back({7, 6, 5, 4});
    // kFaces.push_back({7, 6, 4});
    // kFaces.push_back({6, 5, 4});
    // // kFaces.push_back({0, 7, 4, 3});
    // kFaces.push_back({0, 7, 3});
    // kFaces.push_back({7, 4, 3});
    // // kFaces.push_back({2, 3, 4, 5});
    // kFaces.push_back({2, 3, 5});
    // kFaces.push_back({3, 4, 5});
    // // kFaces.push_back({0, 1, 6, 7});
    // kFaces.push_back({0, 1, 7});
    // kFaces.push_back({1, 6, 7});
    //
    //
    // // cinolib::Polygonmesh<> kMesh("data/cube.off");
    // cinolib::Polygonmesh<> kMesh(kVertices, kFaces);
    //
    // std::cout << "DATOS" << std::endl;
    // std::cout << "Vertices" << std::endl;
    // for (int i = 0; i<kMesh.vector_verts().size(); i++)
    // {
    //     std::cout << i << ": " << kMesh.vector_verts()[i] << std::endl;
    // }
    //
    // std::cout << "Caras" << std::endl;
    // for (int i = 0; i<kMesh.vector_polys().size(); i++)
    // {
    //     std::cout << i << ": ";
    //     for (auto vi : kMesh.vector_polys()[i])
    //     {
    //         std::cout << vi << " ";
    //     }
    //     std::cout << std::endl;
    // }
    // std::cout << "Normales" << std::endl;
    // for (int i = 0; i < kMesh.vector_poly_normals().size(); i++)
    // {
    //     std::cout << i << ": " << kMesh.vector_poly_normals()[i] << std::endl;
    // }
    //
    // k.initialize(kMesh.vector_verts());
    // k.compute(kMesh.vector_verts(), kMesh.vector_polys(), kMesh.vector_poly_normals());


    std::cout << "Done in " << times.execution << " ms" << std::endl;
    std::cout << "Created file: " << writer.outputFile << std::endl;
    return 0;
}
