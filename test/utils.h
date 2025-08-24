#ifndef POLYLLA_TEST_UTILS_H
#define POLYLLA_TEST_UTILS_H

#include <gpolylla/polylla.h>
#include <gtest/gtest.h>

using namespace Polylla;

template <typename T> void checkIn(const std::vector<T> &vec, const T &value, const std::string &info)
{
    ASSERT_TRUE(std::find(vec.begin(), vec.end(), value) != vec.end()) << info;
}

template <typename T>
void checkSimilar(const std::vector<T> &subject, const std::vector<T> &expected, const std::string &info)
{
    ASSERT_EQ(subject.size(), expected.size()) << info;
    for (int i = 0; i < subject.size(); i++)
    {
        checkIn(expected, subject[i], info + "[" + std::to_string(i) + "]");
    }
}

const Mesh BASIC_MESH = {
    .vertices = {Vertex(0, 0, 0), Vertex(0, 0, 1), Vertex(1, 0, 1), Vertex(1, 0, 0), Vertex(0, 1, 0), Vertex(0, 1, 1),
                 Vertex(1, 1, 1), Vertex(1, 1, 0)},
    .faces = {Face({0, 1, 2}, {0, -1}), Face({0, 1, 5}, {0, -1}), Face({2, 3, 0}, {1, -1}), Face({0, 5, 2}, {0, 4}),
              Face({0, 7, 2}, {1, 4}), Face({7, 3, 0}, {1, -1}), Face({5, 4, 0}, {3, -1}), Face({0, 4, 7}, {3, -1}),
              Face({0, 5, 7}, {3, 4}), Face({2, 1, 5}, {0, -1}), Face({2, 3, 7}, {1, -1}), Face({5, 6, 2}, {2, -1}),
              Face({5, 2, 7}, {2, 4}), Face({7, 6, 2}, {2, -1}), Face({7, 4, 5}, {3, -1}), Face({5, 6, 7}, {2, -1})},
    .cells = {Tetrahedron({1, 0, 2, 5}, {0, 1, 3, 9}), Tetrahedron({0, 2, 3, 7}, {2, 5, 4, 10}),
              Tetrahedron({2, 5, 6, 7}, {11, 12, 13, 15}), Tetrahedron({5, 7, 0, 4}, {8, 6, 7, 14}),
              Tetrahedron({5, 2, 0, 7}, {3, 4, 8, 12})},
};

const PolyMesh BASIC_POLY_MESH = {
    .vertices = {Vertex(0, 0, 0), Vertex(0, 0, 1), Vertex(1, 0, 1), Vertex(1, 0, 0), Vertex(0, 1, 0), Vertex(0, 1, 1),
                 Vertex(1, 1, 1), Vertex(1, 1, 0)},
    .faces = {Face({0, 1, 2}, {0, -1}), Face({0, 1, 5}, {0, -1}), Face({2, 3, 0}, {1, -1}), Face({0, 5, 2}, {0, 4}),
              Face({0, 7, 2}, {1, 4}), Face({7, 3, 0}, {1, -1}), Face({5, 4, 0}, {3, -1}), Face({0, 4, 7}, {3, -1}),
              Face({0, 5, 7}, {3, 4}), Face({2, 1, 5}, {0, -1}), Face({2, 3, 7}, {1, -1}), Face({5, 6, 2}, {2, -1}),
              Face({5, 2, 7}, {2, 4}), Face({7, 6, 2}, {2, -1}), Face({7, 4, 5}, {3, -1}), Face({5, 6, 7}, {2, -1})},
    .tetras = {Tetrahedron({1, 0, 2, 5}, {0, 1, 3, 9}), Tetrahedron({0, 2, 3, 7}, {2, 5, 4, 10}),
               Tetrahedron({2, 5, 6, 7}, {11, 12, 13, 15}), Tetrahedron({5, 7, 0, 4}, {8, 6, 7, 14}),
               Tetrahedron({5, 2, 0, 7}, {3, 4, 8, 12})},
    .cells = {Polyhedron({0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2, 5, 6, 7, 9, 10, 11, 13, 14, 15}, {0, 1, 2, 3, 4})}

};

#endif