#include "utils.h"
#include <set>
#include <algorithm>

using namespace Polylla;
using namespace std;

struct Sphere
{
    Vertex center;
    float radius;

    Sphere static circumsphere(int ti, const Mesh &mesh)
    {
        using namespace Eigen;
        Matrix4d A, X, Y, Z;
        const Tetrahedron &tetra = mesh.cells[ti];
        const Vertex &p0 = mesh.vertices[tetra.vertices[0]];
        const Vertex &p1 = mesh.vertices[tetra.vertices[1]];
        const Vertex &p2 = mesh.vertices[tetra.vertices[2]];
        const Vertex &p3 = mesh.vertices[tetra.vertices[3]];

        A << p0.x(), p0.y(), p0.z(), 1, p1.x(), p1.y(), p1.z(), 1, p2.x(), p2.y(), p2.z(), 1, p3.x(), p3.y(), p3.z(), 1;

        X << p0.squaredNorm(), p0.y(), p0.z(), 1, p1.squaredNorm(), p1.y(), p1.z(), 1, p2.squaredNorm(), p2.y(), p2.z(),
            1, p3.squaredNorm(), p3.y(), p3.z(), 1;

        Y << p0.squaredNorm(), p0.x(), p0.z(), 1, p1.squaredNorm(), p1.x(), p1.z(), 1, p2.squaredNorm(), p2.x(), p2.z(),
            1, p3.squaredNorm(), p3.x(), p3.z(), 1;

        Z << p0.squaredNorm(), p0.x(), p0.y(), 1, p1.squaredNorm(), p1.x(), p1.y(), 1, p2.squaredNorm(), p2.x(), p2.y(),
            1, p3.squaredNorm(), p3.x(), p3.y(), 1;

        double a = A.determinant();
        double Dx = X.determinant();
        double Dy = -Y.determinant();
        double Dz = Z.determinant();

        Sphere sphere;
        sphere.center = Vertex(Dx / (2 * a), Dy / (2 * a), Dz / (2 * a));
        sphere.radius = (sphere.center - p0).norm();
        return sphere;
    }

    bool isIn(const Vertex &point) const
    {
        return (center - point).norm() < radius + TOLERANCE;
    }
};

struct CavityInfo
{
    vector<Sphere> cavities;
    vector<int> seeds;
    vector<int> owners;
};

struct DepthFirstSearch
{
    const Mesh *mesh;
    CavityInfo *info;

    DepthFirstSearch(const Mesh *mesh, CavityInfo *info) : mesh(mesh), info(info)
    {
    }

    void operator()(const int currentTi, const int seed, set<int> *points, vector<int> *faces, vector<int> *tetras)
    {
        info->owners[currentTi] = seed;
        tetras->push_back(currentTi);

        const Tetrahedron &currentTetra = mesh->cells[currentTi];

        for (int vi : currentTetra.vertices)
        {
            points->insert(vi);
        }

        for (const int fi : currentTetra.faces)
        {
            int nextTi = mesh->faces[fi].tetras[0];
            if (nextTi == currentTi)
                nextTi = mesh->faces[fi].tetras[1];

            if (nextTi == -1)
            {
                faces->push_back(fi);
                continue;
            }

            if (info->owners[nextTi] != -1)
            {
                // Ya fue asignado a un poly
                if (info->owners[nextTi] != seed)
                    faces->push_back(fi);
                continue;
            }

            const Sphere &cavity = info->cavities[seed];
            if (cavity.isIn(info->cavities[nextTi].center))
                (*this)(nextTi, seed, points, faces, tetras);
            else
                faces->push_back(fi);
        }
    }
};

void labelCavities(const Mesh &mesh, PolyMesh *result, CavityInfo *info)
{
    for (int ti = 0; ti < mesh.cells.size(); ++ti)
    {
        const Sphere &sphere = Sphere::circumsphere(ti, mesh);
        info->cavities.push_back(sphere);
        info->seeds.push_back(ti);
    }

    std::sort(info->seeds.begin(), info->seeds.end(), [&](int i, int j) { return info->cavities[i].radius < info->cavities[j].radius; });

    info->owners = vector<int>(mesh.cells.size(), -1);
}

void buildCavities(const Mesh &mesh, PolyMesh *result, CavityInfo *info) {
    DepthFirstSearch dfs(&mesh, info);

    // Copy vertices from the original mesh
    result->vertices = mesh.vertices;
    result->tetras = mesh.cells;

    for (int ti : info->seeds) {
        if (info->owners[ti] != -1) continue;

        set<int> uniquePoints;
        vector<int> originalFaces;
        vector<int> tetras;
        dfs(ti, ti, &uniquePoints, &originalFaces, &tetras);

        // Only create polyhedron if we have valid data
        if (!uniquePoints.empty() && !tetras.empty()) {
            // Create new faces for this polyhedron
            vector<int> newFaceIndices;
            for (int fi : originalFaces) {
                if (fi >= 0 && fi < static_cast<int>(mesh.faces.size())) {
                    const Face& originalFace = mesh.faces[fi];
                    
                    // Create a new face in the result
                    Face newFace = originalFace;
                    result->faces.push_back(newFace);
                    newFaceIndices.push_back(result->faces.size() - 1);
                }
            }

            // If we have faces, create the polyhedron
            if (!newFaceIndices.empty()) {
                vector<int> points(uniquePoints.begin(), uniquePoints.end());
                result->cells.emplace_back(points, newFaceIndices, tetras);
            }
        }
    }
};

void fixCavities(const Mesh &mesh, PolyMesh *result, CavityInfo *info) {
    // For now, this function is a placeholder
    // In a complete implementation, it would handle:
    // - Boundary face identification
    // - Cavity validation
    // - Edge case handling
    // - Quality checks
};

PolyMesh CavityAlgorithm::operator()(const Mesh &mesh)
{
    PolyMesh result;
    CavityInfo info;
    labelCavities(mesh, &result, &info);
    buildCavities(mesh, &result, &info);
    fixCavities(mesh, &result, &info);
    return result;
}