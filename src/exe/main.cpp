//
// Created by vigb9 on 01-06-2025.
//
#include <gpolylla/polylla.h>
#include <gpolylla/stat.h>
#include <iostream>
#include <polyhedron_kernel.h>

using namespace Polylla;

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        std::cerr << "Usage: " << argv[0] << " <node_file> <ele_file> <output_file>" << std::endl;
    }

    TetgenReader reader;
    reader.nodeFile = argv[1];
    reader.eleFile = argv[2];
    Mesh mesh = reader.readMesh();

    PolyMesh polyMesh = CavityAlgorithm()(mesh);

    auto stats = computeStats(polyMesh);
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


    VisFWriter writer;
    writer.outputFile = argv[3];
    writer.writeMesh(polyMesh);

    std::cout << "Done" << std::endl;
    std::cout << "Created file: " << writer.outputFile << std::endl;
    return 0;
}
