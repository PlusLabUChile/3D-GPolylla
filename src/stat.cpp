//
// Created by vigb9 on 06/10/2025.
//
#include "../cmake-build-debug/_deps/imgui-src/imgui.h"
#include "utils.h"
#include <gpolylla/stat.h>

#include <QuickHull.hpp>

#include <polyhedron_kernel.h>


#include <numeric>

using namespace Polylla;

float generalVolume(const std::vector<Vertex> &vertices, const std::vector<Face> &faces)
{
    Vertex ref = vertices[0];
    float signedVolume = 0.f;
    for (auto f: faces)
    {
        const auto &v0 = vertices[f.vertices[0]];
        const auto &v1 = vertices[f.vertices[1]];
        const auto &v2 = vertices[f.vertices[2]];
        signedVolume += signedSixthVolume(ref, v0, v1, v2);
    }
    return abs(signedVolume) / 6.0f;
}

float generalArea(const std::vector<Vertex> &vertices, const std::vector<Face> &faces)
{
    float totalArea = 0.f;
    for (auto f: faces)
    {
        const auto &v0 = vertices[f.vertices[0]];
        const auto &v1 = vertices[f.vertices[1]];
        const auto &v2 = vertices[f.vertices[2]];
        totalArea += normal(v0, v1, v2).norm();
    }
    return .5f * totalArea;
}


Hull::Hull(const Polyhedron &poly, const PolyMesh &mesh)
{
    quickhull::QuickHull<float> qh;
    std::vector<quickhull::Vector3<float>> qhVertices;
    vertices.reserve(poly.vertices.size());
    qhVertices.reserve(poly.vertices.size());
    for (int vi : poly.vertices)
    {
        const auto& vert = mesh.vertices[vi];
        vertices.push_back(vert);
        qhVertices.emplace_back(vert.x(), vert.y(), vert.z());
    }
    auto hull = qh.getConvexHull(qhVertices, true, true);
    const auto& indices = hull.getIndexBuffer();
    int facesAmount = indices.size() / 3;
    faces.reserve(facesAmount);
    for (int fi = 0; fi < facesAmount; fi++)
    {
        faces.emplace_back(indices[fi * 3], indices[fi * 3 + 1], indices[fi * 3 + 2]);
    }
}

float Hull::volume() const
{
    return generalVolume(vertices, faces);
}

float Hull::area() const
{
    return generalArea(vertices, faces);
}

 Kernel::Kernel(const Polyhedron &poly, const PolyMesh &mesh)
{
    PolyhedronKernel k;
    std::vector<cinolib::vec3d> kVertices;
    std::vector<std::vector<uint>> kFaces;
    std::unordered_map<int, int> vertLookup;
    kVertices.reserve(poly.vertices.size());
    for (int vi : poly.vertices)
    {
        vertLookup[vi] = kVertices.size();
        const auto& vert = mesh.vertices[vi];
        kVertices.emplace_back(vert.x(), vert.y(), vert.z());
    }

    auto directedFaces = getDirectedFaces(poly, mesh);

    for (auto face : directedFaces)
    {
        // const auto& face = mesh.faces[fi];
        std::vector<uint> faceVertices(3);
        for (int i = 0; i < 3; ++i)
        {
            faceVertices[i] = vertLookup[face[i]];
        }
        kFaces.push_back(faceVertices);
    }
    cinolib::Polygonmesh<> m(kVertices, kFaces);
    k.initialize(m.vector_verts());
    auto normals = m.vector_poly_normals();
    k.compute(m.vector_verts(), m.vector_polys(), m.vector_poly_normals());

    vertices.reserve(k.kernel_verts.size());
    for (const auto& v : k.kernel_verts)
    {
        vertices.emplace_back(v.x(), v.y(), v.z());
    }

    // faces.(k.kernel_faces.size());
    for (const auto& f : k.kernel_faces)
    {
        if (f.size() > 3)
        {
            // std::cerr << "Error while computing kernel, face larger than a triangle found (" << f.size() << ")" << std::endl;
            // continue;
            // make a triangle fan from the face
            uint root = f.at(0);
            for (uint i = 1; i <f.size() -1 ; ++i)
            {
                faces.emplace_back(root, f.at(i), f.at(i+1));
            }
            continue;
        }

        faces.emplace_back(f[0], f[1], f[2]);
    }
}


float Kernel::volume() const
{
    return generalVolume(vertices, faces);
}

float Kernel::area() const
{
    return generalArea(vertices, faces);
}


bool Kernel::empty() const
{
    return vertices.empty();
}


//  MeshStat::MeshStat(const PolyMesh &mesh)
// {
//     kernels.reserve(mesh.cells.size());
//     hulls.reserve(mesh.cells.size());
//
//     for (int pi = 0; pi < mesh.cells.size(); ++pi)
//     {
//         const Polyhedron &p = mesh.cells[pi];
//         hulls.push_back(Hull(p, mesh));
//         kernels.push_back(Kernel(p, mesh));
//     }
// }

std::vector<PolyStat> Polylla::computeStats(const PolyMesh &mesh)
{
    std::vector<PolyStat> stats;
    stats.reserve(mesh.cells.size());
    for (const auto& poly : mesh.cells)
    {
        PolyStat stat;
        stat.hull = Hull(poly, mesh);
        Kernel possibleKernel(poly, mesh);

        stat.kernel = possibleKernel;
        if (possibleKernel.empty())
        {
            stat.kernel = std::nullopt;
        }

        float minSize = std::numeric_limits<float>::max();
        float maxSize = std::numeric_limits<float>::min();

        for (const auto& fi : poly.faces)
        {
            const Face &f = mesh.faces[fi];
            for (int i = 0; i < 3; ++i)
            {
                const auto &v0 = mesh.vertices[f.vertices[i]];
                const auto &v1 = mesh.vertices[f.vertices[(i + 1) % 3]];
                float edgeSize = (v1 - v0).norm();
                minSize = std::min(minSize, edgeSize);
                maxSize = std::max(maxSize, edgeSize);
            }
        }

        stat.edgeRatio = minSize / maxSize;
        stat.volumeRatio = 0.0f;
        if (stat.kernel)
            stat.volumeRatio = poly.volume(mesh) / stat.kernel.value().volume();
        stat.surfaceRatio = poly.area(mesh) / stat.hull.area();
        stats.push_back(stat);
    }

    return stats;
}

