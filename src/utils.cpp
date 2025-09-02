#include "utils.h"

#include <cmath>
#include <unordered_map>
#include <unordered_set>

namespace Polylla
{

using vec3 = Eigen::Vector3f;

vec3 normal(const Vertex &v0, const Vertex &v1, const Vertex &v2)
{
    return (v1 - v0).cross(v2 - v0).normalized();
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
        totalArea += 0.5f * (v1 - v0).cross(v2 - v0).norm();
    }
    return totalArea;
}

float polyhedronVolume(const Polyhedron &polyhedron, const PolyMesh &mesh)
{
    float totalVolume = 0.0f;
    std::unordered_set<int> poly_cells(polyhedron.cells.begin(), polyhedron.cells.end());

    for (int face_idx : polyhedron.faces)
    {
        const Face &face = mesh.faces[face_idx];

        int inner_tetra_idx = -1;
        if (face.tetras[0] != -1 && poly_cells.count(face.tetras[0]))
        {
            inner_tetra_idx = face.tetras[0];
        }
        else if (face.tetras[1] != -1 && poly_cells.count(face.tetras[1]))
        {
            inner_tetra_idx = face.tetras[1];
        }
        else
        {
            const Vertex &v0 = mesh.vertices[face.vertices[0]];
            const Vertex &v1 = mesh.vertices[face.vertices[1]];
            const Vertex &v2 = mesh.vertices[face.vertices[2]];
            totalVolume += v0.dot(v1.cross(v2));
            continue;
        }

        const Tetrahedron &inner_tetra = mesh.tetras[inner_tetra_idx];

        int fourth_v_idx = -1;
        for (int i = 0; i < 4; ++i)
        {
            int current_v_idx = inner_tetra.vertices[i];
            if (current_v_idx != face.vertices[0] && current_v_idx != face.vertices[1] &&
                current_v_idx != face.vertices[2])
            {
                fourth_v_idx = current_v_idx;
                break;
            }
        }

        if (fourth_v_idx == -1)
        {
            const Vertex &v0 = mesh.vertices[face.vertices[0]];
            const Vertex &v1 = mesh.vertices[face.vertices[1]];
            const Vertex &v2 = mesh.vertices[face.vertices[2]];
            totalVolume += v0.dot(v1.cross(v2));
            continue;
        }

        const Vertex &p = mesh.vertices[fourth_v_idx];
        const Vertex &v0 = mesh.vertices[face.vertices[0]];
        const Vertex &v1 = mesh.vertices[face.vertices[1]];
        const Vertex &v2 = mesh.vertices[face.vertices[2]];

        if (isOutside(v0, v1, v2, p) > 0)
        {
            totalVolume += v0.dot(v2.cross(v1));
        }
        else
        {
            totalVolume += v0.dot(v1.cross(v2));
        }
    }
    return std::abs(totalVolume / 6.0f);
}

} // namespace Polylla
