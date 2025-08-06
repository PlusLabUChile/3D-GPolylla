#ifndef UTILS_H
#define UTILS_H
#include <gpolylla/polylla.h>

namespace Polylla
{

constexpr int FACE_CONFIGURATION[4][3] = {
    {1, 2, 3}, // 0
    {0, 2, 3}, // 1
    {0, 1, 3}, // 2
    {0, 1, 2}  // 3
};

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

std::vector<Face> buildFaces(const std::vector<Vertex> &vertices, const std::vector<Tetrahedron> &tetrahedrons);

void buildConnectivity(Mesh *mesh);
} // namespace Polylla

#endif // UTILS_H
