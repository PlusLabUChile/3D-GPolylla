#include <gpolylla/polylla.h>

#include <algorithm>

using namespace Polylla;
using namespace std;

// Face constructors
Face::Face()
{
    // Default initialization already handled by member initializers
}

Face::Face(int v0, int v1, int v2)
{
    vertices = {v0, v1, v2};
}

Face::Face(const std::array<int, 3> &verts) : vertices(verts)
{
}

Face::Face(const std::array<int, 3> &verts, const std::array<int, 2> &tets) : vertices(verts), tetras(tets)
{
}

// Tetrahedron constructors
Tetrahedron::Tetrahedron()
{
    // Default initialization already handled by member initializers
}

Tetrahedron::Tetrahedron(int v0, int v1, int v2, int v3)
{
    vertices = {v0, v1, v2, v3};
}

Tetrahedron::Tetrahedron(const std::array<int, 4> &verts) : vertices(verts)
{
}

Tetrahedron::Tetrahedron(const std::array<int, 4> &verts, const std::array<int, 4> &faces)
    : vertices(verts), faces(faces)
{
}

// Polyhedron constructors
Polyhedron::Polyhedron()
{
    // Default initialization already handled
}

Polyhedron::Polyhedron(const std::vector<int> &verts) : vertices(verts)
{
}

Polyhedron::Polyhedron(const std::vector<int> &verts, const vector<int> &faces, const std::vector<int> &cellIds)
    : vertices(verts), faces(faces), cells(cellIds)
{
}

// Hash implementations
std::size_t Face::hash() const
{
    // Create a sorted copy of vertices for order-independent hashing
    std::array<int, 3> sorted_vertices = vertices;
    std::ranges::sort(sorted_vertices);

    std::size_t h1 = std::hash<int>{}(sorted_vertices[0]);
    std::size_t h2 = std::hash<int>{}(sorted_vertices[1]);
    std::size_t h3 = std::hash<int>{}(sorted_vertices[2]);

    // Combine hashes using XOR and bit shifting for better distribution
    return h1 ^ (h2 << 1) ^ (h3 << 2);
}

std::size_t Tetrahedron::hash() const
{
    // Create a sorted copy of vertices for order-independent hashing
    std::array<int, 4> sorted_vertices = vertices;
    std::ranges::sort(sorted_vertices);

    std::size_t h1 = std::hash<int>{}(sorted_vertices[0]);
    std::size_t h2 = std::hash<int>{}(sorted_vertices[1]);
    std::size_t h3 = std::hash<int>{}(sorted_vertices[2]);
    std::size_t h4 = std::hash<int>{}(sorted_vertices[3]);

    // Combine hashes using XOR and bit shifting for better distribution
    return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3);
}

std::size_t Polyhedron::hash() const
{
    // Create a sorted copy of vertices for order-independent hashing
    std::vector<int> sorted_vertices = vertices;
    std::ranges::sort(sorted_vertices);

    std::size_t result = 0;
    for (std::size_t i = 0; i < sorted_vertices.size(); ++i)
    {
        result ^= std::hash<int>{}(sorted_vertices[i]) << (i % 8);
    }

    return result;
}

// Equality implementations
bool Face::operator==(const Face &other) const
{
    // Create sorted copies of vertices for order-independent comparison
    std::array<int, 3> sorted_vertices = vertices;
    std::array<int, 3> other_sorted_vertices = other.vertices;
    std::ranges::sort(sorted_vertices);
    std::ranges::sort(other_sorted_vertices);
    std::array<int, 2> sorted_tetras = tetras;
    std::array<int, 2> other_sorted_tetras = other.tetras;

    std::ranges::sort(sorted_tetras);
    std::ranges::sort(other_sorted_tetras);

    return sorted_vertices == other_sorted_vertices && sorted_tetras == other_sorted_tetras;
}

bool Tetrahedron::operator==(const Tetrahedron &other) const
{
    // Create sorted copies of vertices for order-independent comparison
    std::array<int, 4> sorted_vertices = vertices;
    std::array<int, 4> other_sorted_vertices = other.vertices;

    std::ranges::sort(sorted_vertices);
    std::ranges::sort(other_sorted_vertices);

    std::array<int, 4> sorted_faces = faces;
    std::array<int, 4> other_sorted_faces = other.faces;

    std::ranges::sort(sorted_faces);
    std::ranges::sort(other_sorted_faces);

    return sorted_vertices == other_sorted_vertices && sorted_faces == other_sorted_faces;
}

bool Polyhedron::operator==(const Polyhedron &other) const
{
    // Create sorted copies of vertices for order-independent comparison
    std::vector<int> sorted_vertices = vertices;
    std::vector<int> other_sorted_vertices = other.vertices;

    std::ranges::sort(sorted_vertices);
    std::ranges::sort(other_sorted_vertices);

    std::vector<int> sorted_cells = cells;
    std::vector<int> other_sorted_cells = other.cells;

    std::ranges::sort(sorted_cells);
    std::ranges::sort(other_sorted_cells);

    std::vector<int> sorted_faces = faces;
    std::vector<int> other_sorted_faces = other.faces;

    std::ranges::sort(sorted_faces);
    std::ranges::sort(other_sorted_faces);

    return sorted_vertices == other_sorted_vertices && sorted_faces == other_sorted_faces &&
           sorted_cells == other_sorted_cells;
}
