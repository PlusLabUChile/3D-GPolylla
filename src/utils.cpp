#include "utils.h"

#include "ConvexHull.hpp"
#include "QuickHull.hpp"

#include <cmath>
#include <unordered_map>
#include <unordered_set>

namespace Polylla
{

using vec3 = Eigen::Vector3f;
using namespace std;

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
float Hull::area() const
{
    // garanteed to be CCW consistent
     float totalArea = 0.0f;
     for (const auto& face : faces)
     {
         const Vertex &v0 = vertices[face.vertices[0]];
         const Vertex &v1 = vertices[face.vertices[1]];
         const Vertex &v2 = vertices[face.vertices[2]];
         totalArea += 0.5f * normal(v0, v1, v2).norm();
     }
     return totalArea;
}

float Hull::volume() const
{
     float totalVolume = 0.0f;
     for (const auto& face : faces)
     {
         const Vertex &v0 = vertices[face.vertices[0]];
         const Vertex &v1 = vertices[face.vertices[1]];
         const Vertex &v2 = vertices[face.vertices[2]];
         totalVolume += v0.dot(normal(v0, v1, v2));
     }
     return abs(totalVolume / 6.0f);
}




int opposite(const Face &f, const Tetrahedron &tetra)
{
    // the opposite of a face with respect a tetra
    // is the only point that its not part of the face
    for (int vi : tetra.vertices)
    {
        if (ranges::find(f.vertices, vi) == f.vertices.end())
            return vi;
    }
    return -1;
}


vec3 normal(const Vertex &v0, const Vertex &v1, const Vertex &v2)
{
    return (v1 - v0).cross(v2 - v0);
}


float isOutside(const Vertex &v0, const Vertex &v1, const Vertex &v2, const Vertex &other)
{
    vec3 n = normal(v0, v1, v2);
    return (other - v0).dot(n);
}

float polyhedronArea(const Polyhedron &polyhedron, const PolyMesh &mesh)
{
    float totalArea = 0.0f;
    for (int face_idx : polyhedron.faces)
    {
        const Face &face = mesh.faces[face_idx];
        const Vertex &v0 = mesh.vertices[face.vertices[0]];
        const Vertex &v1 = mesh.vertices[face.vertices[1]];
        const Vertex &v2 = mesh.vertices[face.vertices[2]];
        totalArea += 0.5f * normal(v0, v1, v2).norm();
    }
    return totalArea;
}

float polyhedronVolume(const Polyhedron &polyhedron, const PolyMesh &mesh)
{
    // asumimos polyhedros bien formados
    float totalVolume = 0.0f;
    std::unordered_set<int> polyCells(polyhedron.cells.begin(), polyhedron.cells.end());

    for (int faceIdx : polyhedron.faces)
    {
        const Face &face = mesh.faces[faceIdx];

        // no faces should be {-1, -1} at tetras
        int inneTetraIdx = face.tetras[0];
        if (face.tetras[1] != -1 && polyCells.contains(face.tetras[1]))
        {
            inneTetraIdx = face.tetras[1];
        }

        const Tetrahedron &innerTetra = mesh.tetras[inneTetraIdx];

        int fourth = opposite(face, innerTetra);
        // int fourth_v_idx = -1;
        // for (int i = 0; i < 4; ++i)
        // {
        //     int current_v_idx = innerTetra.vertices[i];
        //     if (current_v_idx != face.vertices[0] && current_v_idx != face.vertices[1] &&
        //         current_v_idx != face.vertices[2])
        //     {
        //         fourth_v_idx = current_v_idx;
        //         break;
        //     }
        // }

        // if (fourth_v_idx == -1)
        // {
        //     const Vertex &v0 = mesh.vertices[face.vertices[0]];
        //     const Vertex &v1 = mesh.vertices[face.vertices[1]];
        //     const Vertex &v2 = mesh.vertices[face.vertices[2]];
        //     totalVolume += v0.dot(v1.cross(v2));
        //     continue;
        // }

        const Vertex &p = mesh.vertices[fourth];
        const Vertex &v0 = mesh.vertices[face.vertices[0]];
        const Vertex &v1 = mesh.vertices[face.vertices[1]];
        const Vertex &v2 = mesh.vertices[face.vertices[2]];

        if (isOutside(v0, v1, v2, p) > 0)
        {
            totalVolume += v0.dot(normal(v0, v1, v2));
        }
        else
        {
            totalVolume += v0.dot(normal(v0, v2, v1));
        }
    }
    return std::abs(totalVolume / 6.0f);
}

} // namespace Polylla
