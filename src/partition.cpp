#include <algorithm>
#include <gpolylla/partition.h>
#include <map>
#include <queue>
#include <set>
#include <stdexcept>
using namespace Polylla;
PolyMesh Polylla::fromMembership(const Mesh &mesh, const std::vector<int> &membership)
{
    if (membership.size() != mesh.tetras.size())
        throw std::runtime_error("Membership length mismatch");
    PolyMesh result;
    static_cast<Mesh &>(result) = mesh;
    std::map<int, int> ids;
    for (int ti = 0; ti < membership.size(); ++ti)
    {
        int id = membership[ti];
        if (id < 0)
            throw std::runtime_error("Negative membership ID");
        if (!ids.contains(id))
        {
            ids[id] = result.cells.size();
            result.cells.emplace_back();
        }
        int pi = ids.at(id);
        result.tetras[ti].polyhedron = pi;
        result.cells[pi].cells.push_back(ti);
    }
    for (int pi = 0; pi < result.cells.size(); ++pi)
    {
        auto &poly = result.cells[pi];
        std::set<int> vertices;
        for (int ti : poly.cells)
        {
            const auto &tetra = mesh.tetras[ti];
            vertices.insert(tetra.vertices.begin(), tetra.vertices.end());
            for (int fi : tetra.faces)
            {
                const auto &face = mesh.faces[fi];
                int next = face.tetras[0] == ti ? face.tetras[1] : face.tetras[0];
                if (next < 0 || result.tetras[next].polyhedron != pi)
                    poly.faces.push_back(fi);
            }
        }
        poly.vertices.assign(vertices.begin(), vertices.end());
    }
    validatePartition(result);
    return result;
}
void Polylla::validatePartition(const PolyMesh &mesh)
{
    std::vector<int> seen(mesh.tetras.size(), 0);
    for (int pi = 0; pi < mesh.cells.size(); ++pi)
    {
        const auto &poly = mesh.cells[pi];
        if (poly.cells.empty())
            throw std::runtime_error("Empty cell");
        std::map<int, int> counts;
        std::set<int> members(poly.cells.begin(), poly.cells.end()), reached;
        for (int ti : poly.cells)
        {
            if (ti < 0 || ti >= seen.size() || ++seen[ti] != 1 || mesh.tetras[ti].polyhedron != pi)
                throw std::runtime_error("Partition exclusivity or owner mismatch");
            for (int fi : mesh.tetras[ti].faces)
                ++counts[fi];
        }
        std::set<int> expected;
        for (auto [fi, count] : counts)
            if (count == 1)
                expected.insert(fi);
        std::set<int> actual(poly.faces.begin(), poly.faces.end());
        if (actual != expected || actual.size() != poly.faces.size())
            throw std::runtime_error("Incorrect cell boundary");
        std::queue<int> pending;
        pending.push(poly.cells.front());
        reached.insert(poly.cells.front());
        while (!pending.empty())
        {
            int ti = pending.front();
            pending.pop();
            for (int fi : mesh.tetras[ti].faces)
                for (int next : mesh.faces[fi].tetras)
                    if (members.contains(next) && reached.insert(next).second)
                        pending.push(next);
        }
        if (reached.size() != members.size())
            throw std::runtime_error("Disconnected cell");
    }
    if (std::find(seen.begin(), seen.end(), 0) != seen.end())
        throw std::runtime_error("Incomplete partition");
}
