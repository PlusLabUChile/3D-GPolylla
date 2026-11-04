#include "QuickHull.hpp"
#include "utils.h"
#include <algorithm>
#include <random>
#include <set>
#include <stdexcept>

using namespace Polylla;
using namespace std;

CavityAlgorithm::Cavity circumsphere(int ti, const Mesh &mesh)
{
    const auto &t = mesh.tetras.at(ti);
    const Vertex &p = mesh.vertices.at(t.vertices[0]);
    Eigen::Matrix3d a;
    Eigen::Vector3d rhs;
    for (int i = 0; i < 3; ++i)
    {
        Eigen::Vector3d d = mesh.vertices.at(t.vertices[i + 1]) - p;
        a.row(i) = 2.0 * d.transpose();
        rhs[i] = d.squaredNorm();
    }
    Eigen::FullPivLU<Eigen::Matrix3d> solver(a);
    if (!solver.isInvertible())
        throw std::runtime_error("Degenerate tetrahedron: " + std::to_string(ti));
    Eigen::Vector3d offset = solver.solve(rhs);
    CavityAlgorithm::Cavity sphere;
    sphere.center = p + offset;
    sphere.radius = offset.norm();
    sphere.tetra = ti;
    if (!sphere.center.allFinite() || !std::isfinite(sphere.radius))
        throw std::runtime_error("Non-finite circumsphere: " + std::to_string(ti));
    return sphere;
}

bool CavityAlgorithm::Cavity::isInside(const Vertex &point) const
{
    return (center - point).norm() <= radius * (1.0 + 1e-10);
}

struct CavityInfo
{
    vector<CavityAlgorithm::Cavity> cavities;
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
        // An explicit DFS stack avoids stack overflow on large connected cavities.
        struct Frame
        {
            int tetra;
            int face;
        };
        std::vector<Frame> stack;
        auto enter = [&](int ti) {
            info->owners[ti] = seed;
            tetras->push_back(ti);
            for (int vi : mesh->tetras[ti].vertices)
                points->insert(vi);
            stack.push_back({ti, 0});
        };
        enter(currentTi);
        while (!stack.empty())
        {
            auto &frame = stack.back();
            if (frame.face == 4)
            {
                stack.pop_back();
                continue;
            }
            int fi = mesh->tetras[frame.tetra].faces[frame.face++];
            const auto &face = mesh->faces[fi];
            int next = face.tetras[0] == frame.tetra ? face.tetras[1] : face.tetras[0];
            if (next == -1)
            {
                faces->push_back(fi);
                continue;
            }
            if (info->owners[next] != -1)
            {
                if (info->owners[next] != seed)
                    faces->push_back(fi);
                continue;
            }
            if (info->cavities[seed].isInside(info->cavities[next].center))
                enter(next);
            else
                faces->push_back(fi);
        }
    }
};

void labelCavities(const Mesh &mesh, PolyMesh *result, CavityInfo *info)
{
    for (int ti = 0; ti < mesh.tetras.size(); ++ti)
    {
        const auto &sphere = circumsphere(ti, mesh);
        info->cavities.push_back(sphere);
        info->seeds.push_back(ti);
    }

    std::sort(info->seeds.begin(), info->seeds.end(), [&](int i, int j) {
        double a = info->cavities[i].radius, b = info->cavities[j].radius;
        return a == b ? i < j : a < b;
    });

    info->owners = vector<int>(mesh.tetras.size(), -1);
}

void buildCavities(const Mesh &mesh, PolyMesh *result, CavityInfo *info)
{
    DepthFirstSearch dfs(&mesh, info);

    // Copy vertices from the original mesh
    result->vertices = mesh.vertices;
    result->faces = mesh.faces;
    result->tetras = mesh.tetras;

    for (int ti : info->seeds)
    {
        if (info->owners[ti] != -1)
            continue;

        set<int> uniquePoints;
        vector<int> faces;
        vector<int> tetras;
        dfs(ti, ti, &uniquePoints, &faces, &tetras);

        vector<int> points(uniquePoints.begin(), uniquePoints.end());
        for (int ti : tetras)
        {
            result->tetras[ti].polyhedron = result->cells.size();
        }
        result->cells.emplace_back(points, faces, tetras);
    }
};

void fixCavities(const Mesh &mesh, PolyMesh *result, CavityInfo *info)
{
    // Add the loners to the best neighbour
    for (const auto &p : result->cells)
    {
        if (p.cells.size() > 1)
            continue;

        int ti = p.cells.at(0);
        const auto &t = result->tetras.at(ti);
        const auto &cavity = info->cavities.at(ti);

        int best = -1;
        float bestValue = numeric_limits<float>::max();

        for (int fi : t.faces)
        {
            const auto &f = result->faces.at(fi);
            int nextTi = f.tetras[0];
            if (nextTi == ti)
                nextTi = f.tetras[1];

            if (nextTi == -1)
                continue;

            // const auto& next = result->cells.at(nextTi);

            const auto &nextCavity = info->cavities.at(nextTi);

            float distance = (nextCavity.center - cavity.center).norm();
            float value = distance / nextCavity.radius;

            if (value < bestValue)
            {
                bestValue = value;
                best = nextTi;
            }
        }

        if (best != -1)
        {
            int pi = result->tetras.at(best).polyhedron;
            result->cells.at(pi).cells.push_back(ti);
            result->tetras.at(ti).polyhedron = pi;
        }
    }

    vector<Polyhedron> newPolys;
    for (auto &t : result->tetras)
    {
        int idx = newPolys.size();
        newPolys.push_back(result->cells.at(t.polyhedron));
        t.polyhedron = idx;
    }
    result->cells = newPolys;
    // For now, this function is a placeholder
    // In a complete implementation, it would handle:
    // - Boundary face identification
    // - Cavity validation
    // - Edge case handling
    // - Quality checks
};
//
// CavityAlgorithm::Information getInfo(const CavityInfo &src, const PolyMesh &mesh)
// {
//
//     CavityAlgorithm::Information info;
//     info.cavity.centers.reserve(src.cavities.size());
//     info.cavity.radius.reserve(src.cavities.size());
//     for (auto [center, radius] : src.cavities)
//     {
//         info.cavity.centers.push_back(center);
//         info.cavity.radius.push_back(radius);
//     }
//
//     info.poly.hullVolumes.reserve(mesh.cells.size());
//     info.poly.volumes.reserve(mesh.cells.size());
//     info.poly.hullAreas.reserve(mesh.cells.size());
//     info.poly.areas.reserve(mesh.cells.size());
//     info.poly.seeds = src.owners;
//     for (const auto& poly : mesh.cells)
//     {
//         Hull hull(poly, mesh);
//         info.poly.hullVolumes.push_back(hull.volume());
//         info.poly.hullAreas.push_back(hull.area());
//         info.poly.areas.push_back(polyhedronArea(poly, mesh));
//         info.poly.volumes.push_back(polyhedronVolume(poly, mesh));
//     }
//     return info;
// }

PolyMesh CavityAlgorithm::operator()(const Mesh &mesh)
{
    PolyMesh result;
    CavityInfo info;
    labelCavities(mesh, &result, &info);
    if (seedOrder == SeedOrder::Descending)
        std::sort(info.seeds.begin(), info.seeds.end(), [&](int i, int j) {
            double a = info.cavities[i].radius, b = info.cavities[j].radius;
            return a == b ? i < j : a > b;
        });
    else if (seedOrder == SeedOrder::Index)
        std::iota(info.seeds.begin(), info.seeds.end(), 0);
    else if (seedOrder == SeedOrder::Random)
    {
        std::iota(info.seeds.begin(), info.seeds.end(), 0);
        std::mt19937 rng(randomSeed);
        std::shuffle(info.seeds.begin(), info.seeds.end(), rng);
    }
    buildCavities(mesh, &result, &info);
    // fixCavities(mesh, &result, &info);
    // if (withInfo)
    // {
    //     this->info = getInfo(info, result);
    // }
    // owners = info.owners;
    // Reconstruct boundary after all owners are known: a rejected neighbour may
    // subsequently enter the same cavity through another face.
    for (auto &cell : result.cells)
    {
        cell.faces.clear();
        for (int ti : cell.cells)
            for (int fi : mesh.tetras[ti].faces)
            {
                const auto &face = mesh.faces[fi];
                int next = face.tetras[0] == ti ? face.tetras[1] : face.tetras[0];
                if (next < 0 || result.tetras[next].polyhedron != result.tetras[ti].polyhedron)
                    cell.faces.push_back(fi);
            }
    }
    cavities_ = info.cavities;
    seeds_ = info.seeds;
    owners_ = info.owners;
    return result;
}