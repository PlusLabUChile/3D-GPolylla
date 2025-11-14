#ifndef UTILS_H
#define UTILS_H
#include <gpolylla/polylla.h>
#include <numeric>

namespace Polylla
{
// class Hull
// {
//     std::vector<Face> faces;
//     std::vector<Vertex> vertices;
// public:
//     Hull(const Polyhedron& poly, const PolyMesh& mesh);
//     float area() const;
//     float volume() const;
// };

constexpr int FACE_CONFIGURATION[4][3] = {
    {1, 2, 3}, // 0
    {0, 2, 3}, // 1
    {0, 1, 3}, // 2
    {0, 1, 2}  // 3
};

constexpr float TOLERANCE = 0.00000001f;

template <typename T> bool sameContent(const T *a, const T *b, size_t size)
{
    bool same = true;
    for (size_t i = 0; i < size; ++i)
    {
        bool equal = false;
        for (size_t j = 0; j < size; ++j)
        {
            if (a[i] == b[j])
            {
                equal = true;
                break;
            }
        }

        if (!equal)
        {
            same = false;
            break;
        }
    }
    return same;
}
inline Eigen::Vector3f normal(const Vertex &v0, const Vertex &v1, const Vertex &v2)
{
    return (v1 - v0).cross(v2 - v0);
}

inline float signedSixthVolume(const Vertex &v0, const Vertex &v1, const Vertex &v2, const Vertex &v3)
{
    return (v1 - v0).cross(v2 - v0).dot(v3 - v0);
}

inline bool isOutside(const Vertex &v0, const Vertex &v1, const Vertex &v2, const Vertex &other)
{
    auto n = normal(v0, v1, v2);
    return (other - v0).dot(n) <= 0;
}
inline int opposite(const Face &f, const Tetrahedron &tetra)
{

    for (int vi : tetra.vertices)
    {
        if (std::ranges::find(f.vertices, vi) == f.vertices.end())
            return vi;
    }
    return -1;
}

inline std::vector<std::array<int, 3>> getDirectedFaces(const Polyhedron& p, const PolyMesh& mesh)
{
    std::vector<std::array<int, 3>> faces;
    for (const int ti : p.cells)
    {
        const Tetrahedron &t = mesh.tetras[ti];
        for (const int tetraFi : t.faces)
        {

            if (std::ranges::find(p.faces, tetraFi) == p.faces.end())
                continue;

            const Face &f = mesh.faces[tetraFi];
            for (const int ref : t.vertices)
            {
                // Ref is the only vertex that is not part of the face
                if (std::ranges::find(f.vertices, ref) != f.vertices.end())
                    continue;

                // Direct the face based on the reference vertex
                auto vertices = f.vertices;
                const Vertex &other = mesh.vertices[ref];
                const Vertex &v0 = mesh.vertices[mesh.faces[tetraFi].vertices[0]];
                const Vertex &v1 = mesh.vertices[mesh.faces[tetraFi].vertices[1]];
                const Vertex &v2 = mesh.vertices[mesh.faces[tetraFi].vertices[2]];
                if (!isOutside(v0, v1, v2, other))
                {
                    // If the reference vertex is inside, reverse the order
                    std::ranges::reverse(vertices);
                }
                faces.push_back(vertices);
                break;
            }
        }
    }

    return faces;
}

// inline float edgeSize(const Vertex v0, const Vertex v1)
// {
//     return (v1 - v0).norm();
// }
//
// inline float maxEdgeSize(const Face& f, const Mesh& m)
// {
//
//     float max = 0;
//     for (int i = 0; i < 3; i++)
//     {
//         const auto& v0 = m.vertices.at(f.vertices[i]);
//         const auto& v1 = m.vertices.at(f.vertices[(i + 1) % 3]);
//         float size = edgeSize(v0, v1);
//     }
// }
//
// inline float minEdgeSize(const Face& f, const Mesh& m)
// {
//
}

// Hull polyhedronHull(const Polyhedron &polyhedron, const PolyMesh &mesh);
// } // namespace Polylla

#endif // UTILS_H
