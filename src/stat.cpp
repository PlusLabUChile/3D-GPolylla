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
    return 1/6 * abs(signedVolume);
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
    return .5 * totalArea;
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
    kVertices.reserve(poly.vertices.size());
    for (int vi : poly.vertices)
    {
        const auto& vert = mesh.vertices[vi];
        vertices.push_back(vert);
        kVertices.emplace_back(vert.x(), vert.y(), vert.z());
    }

    for (int fi : poly.faces)
    {
        const auto& face = mesh.faces[fi];
        kFaces.emplace_back(face.vertices.begin(), face.vertices.end());
    }
    cinolib::Polygonmesh<> m(kVertices, kFaces);
    k.initialize(m.vector_verts());
    k.compute(m.vector_verts(), m.vector_polys(), m.vector_poly_normals());

    vertices.reserve(k.kernel_verts.size());
    for (const auto& v : k.kernel_verts)
    {
        vertices.emplace_back(v.x(), v.y(), v.z());
    }

    faces.reserve(k.kernel_faces.size());
    for (const auto& f : k.kernel_faces)
    {
        if (f.size() != 3)
        {
            std::cerr << "Error while computing kernel, face larger than a triangle found" << std::endl;
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

std::vector<PolyStat> computeStats(const PolyMesh &mesh)
{
    std::vector<PolyStat> stats;
    stats.reserve(mesh.cells.size());
    for (const auto& poly : mesh.cells)
    {
        PolyStat stat;
        stat.hull = Hull(poly, mesh);
        stat.kernel = Kernel(poly, mesh);

        float minSize = std::numeric_limits<float>::max();
        float maxSize = std::numeric_limits<float>::min();

        for (const auto& fi : poly.faces)
        {
            const Face &f = mesh.faces[fi];
            for (int i = 0; i < 3; ++i)
            {
                const auto &v0 = mesh.vertices[f.vertices[i]];
                const auto &v1 = mesh.vertices[f.vertices[(i + 1) % 3]];
                float edgeSize = (v1 - v0).size();
                minSize = std::min(minSize, edgeSize);
                maxSize = std::max(maxSize, edgeSize);
            }
        }

        stat.edgeRatio = minSize / maxSize;
        stat.volumeRatio = poly.volume(mesh) / stat.hull.volume();
        stat.surfaceRatio = poly.area(mesh) / stat.hull.area();


    }

    return stats;
}
