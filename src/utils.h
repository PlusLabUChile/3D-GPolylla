#ifndef UTILS_H
#define UTILS_H
#include <gpolylla/polylla.h>

namespace Polylla
{
class Hull
{
    std::vector<Face> faces;
    std::vector<Vertex> vertices;
public:
    Hull(const Polyhedron& poly, const PolyMesh& mesh);
    float area() const;
    float volume() const;
};

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
Eigen::Vector3f normal(const Vertex &v0, const Vertex &v1, const Vertex &v2);
float isOutside(const Vertex &v0, const Vertex &v1, const Vertex &v2, const Vertex &other);
int opposite(const Face &f, const Tetrahedron &tetra);

float polyhedronArea(const Polyhedron &polyhedron, const PolyMesh &mesh);
float polyhedronVolume(const Polyhedron &polyhedron, const PolyMesh &mesh);

// Hull polyhedronHull(const Polyhedron &polyhedron, const PolyMesh &mesh);
} // namespace Polylla

#endif // UTILS_H
