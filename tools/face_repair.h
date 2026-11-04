#ifndef GPOLYLLA_EXPERIMENT_FACE_REPAIR_H
#define GPOLYLLA_EXPERIMENT_FACE_REPAIR_H
#include <algorithm>
#include <gpolylla/polylla.h>
#include <map>
#include <set>
#include <stdexcept>
#include <vector>

// Direct port of the reference Face repair, using the same canonical edge-fan
// adapter as run_baseline.py. Invalid reference output is rejected, not patched.
inline std::vector<int> repairedFaceMembership(const Polylla::Mesh &mesh, const std::vector<int> &best)
{
    std::map<std::array<int, 2>, int> edgeIds;
    for (const auto &face : mesh.faces)
        for (int i = 0; i < 3; ++i)
        {
            std::array<int, 2> e = {face.vertices[i], face.vertices[(i + 1) % 3]};
            std::sort(e.begin(), e.end());
            edgeIds[e] = 0;
        }
    int eid = 0;
    for (auto &[key, id] : edgeIds)
        id = eid++;
    std::vector<std::vector<int>> edgeFaces(eid), faceEdges(mesh.faces.size()), orderedFaces(eid);
    std::vector<std::array<int, 4>> tetraFaces;
    for (const auto &tetra : mesh.tetras)
    {
        auto faces = tetra.faces;
        std::sort(faces.begin(), faces.end());
        tetraFaces.push_back(faces);
    }
    for (int fi = 0; fi < mesh.faces.size(); ++fi)
    {
        const auto &f = mesh.faces[fi];
        for (int i = 0; i < 3; ++i)
        {
            std::array<int, 2> e = {f.vertices[i], f.vertices[(i + 1) % 3]};
            std::sort(e.begin(), e.end());
            int id = edgeIds.at(e);
            faceEdges[fi].push_back(id);
            edgeFaces[id].push_back(fi);
        }
    }
    for (int e = 0; e < eid; ++e)
    {
        std::map<int, std::vector<std::pair<int, int>>> adjacent;
        std::set<int> incident, boundary;
        for (int fi : edgeFaces[e])
        {
            const auto &f = mesh.faces[fi];
            int a = f.tetras[0], b = f.tetras[1];
            if (a >= 0)
                incident.insert(a);
            if (b >= 0)
                incident.insert(b);
            if (a < 0 || b < 0)
                boundary.insert(std::max(a, b));
            else
            {
                adjacent[a].push_back({b, fi});
                adjacent[b].push_back({a, fi});
            }
        }
        int start = boundary.empty() ? *incident.begin() : *boundary.begin(), current = start;
        std::set<int> seen;
        std::vector<int> order, path;
        while (!seen.contains(current))
        {
            seen.insert(current);
            order.push_back(current);
            auto choices = adjacent[current];
            std::sort(choices.begin(), choices.end());
            auto next =
                std::find_if(choices.begin(), choices.end(), [&](auto entry) { return !seen.contains(entry.first); });
            if (next == choices.end())
                break;
            current = next->first;
            path.push_back(next->second);
        }
        if (order.size() != incident.size())
            throw std::runtime_error("Disconnected edge fan");
        if (!boundary.empty())
        {
            std::vector<int> first, last;
            for (int fi : edgeFaces[e])
                if (mesh.faces[fi].tetras[1] < 0)
                {
                    if (mesh.faces[fi].tetras[0] == order.front())
                        first.push_back(fi);
                    if (mesh.faces[fi].tetras[0] == order.back())
                        last.push_back(fi);
                }
            orderedFaces[e].push_back(first.front());
            orderedFaces[e].insert(orderedFaces[e].end(), path.begin(), path.end());
            orderedFaces[e].push_back(last.back());
        }
        else
        {
            orderedFaces[e] = path;
            for (auto [n, fi] : adjacent[order.back()])
                if (n == start)
                {
                    orderedFaces[e].push_back(fi);
                    break;
                }
        }
        auto check = orderedFaces[e];
        std::sort(check.begin(), check.end());
        if (check != edgeFaces[e])
            orderedFaces[e] = edgeFaces[e];
    }
    std::vector<bool> frontier(mesh.faces.size()), visited(mesh.tetras.size()), flags(mesh.tetras.size());
    std::vector<int> seeds, owner(mesh.tetras.size(), -1);
    for (int fi = 0; fi < mesh.faces.size(); ++fi)
    {
        int a = mesh.faces[fi].tetras[0], b = mesh.faces[fi].tetras[1];
        frontier[fi] = b < 0 || (best[a] != fi && best[b] != fi);
        if ((b < 0 && best[a] == fi) || (b >= 0 && best[a] == fi && best[b] == fi))
            seeds.push_back(a);
    }
    auto dfs = [&](int seed, std::vector<int> &cells, std::vector<int> &faces, bool repairing) {
        struct Frame
        {
            int ti;
            int index;
        };
        std::vector<Frame> stack;
        auto enter = [&](int ti) {
            if (ti < 0)
                throw std::runtime_error("Invalid repair seed");
            visited[ti] = true;
            if (repairing)
                flags[ti] = false;
            cells.push_back(ti);
            stack.push_back({ti, 0});
        };
        enter(seed);
        while (!stack.empty())
        {
            auto &frame = stack.back();
            if (frame.index == 4)
            {
                stack.pop_back();
                continue;
            }
            int fi = tetraFaces[frame.ti][frame.index++];
            if (frontier[fi])
            {
                faces.push_back(fi);
                continue;
            }
            const auto &f = mesh.faces[fi];
            int next = f.tetras[0] == frame.ti ? f.tetras[1] : f.tetras[0];
            if (next < 0)
                throw std::runtime_error("Invalid frontier");
            if (!visited[next])
                enter(next);
        }
    };
    int pi = 0;
    auto append = [&](const std::vector<int> &cells) {
        for (int ti : cells)
        {
            if (owner[ti] >= 0)
                throw std::runtime_error("Reference repair duplicates tetrahedron");
            owner[ti] = pi;
        }
        ++pi;
    };
    for (int seed : seeds)
    {
        std::vector<int> cells, faces;
        dfs(seed, cells, faces, false);
        std::map<int, int> counts;
        for (int fi : faces)
            ++counts[fi];
        std::set<int> barriers, possibleEdges, uniqueFaces(faces.begin(), faces.end());
        for (auto [fi, count] : counts)
            if (count > 1)
            {
                barriers.insert(fi);
                possibleEdges.insert(faceEdges[fi].begin(), faceEdges[fi].end());
            }
        if (barriers.empty())
        {
            append(cells);
            continue;
        }
        for (int ti : cells)
            visited[ti] = false;
        std::vector<int> pending;
        for (int e : possibleEdges)
        {
            int intersection = 0;
            for (int fi : edgeFaces[e])
                if (uniqueFaces.contains(fi))
                    ++intersection;
            if (intersection != 1)
                continue;
            auto barrier = std::find_if(faces.begin(), faces.end(), [&](int fi) {
                return std::find(faceEdges[fi].begin(), faceEdges[fi].end(), e) != faceEdges[fi].end();
            });
            int bf = *barrier;
            const auto &fan = orderedFaces[e];
            int internal = fan.size() - 1;
            if (internal <= 0)
                throw std::runtime_error("Invalid repair edge fan");
            int advance = internal % 2 == 0 ? internal / 2 - 1 : internal / 2;
            auto position = std::find(fan.begin(), fan.end(), bf);
            if (position == fan.end())
                throw std::runtime_error("Missing repair barrier face");
            int middle = fan[(position - fan.begin() + 1 + advance) % internal];
            if (middle == bf)
                throw std::runtime_error("Reference repair cannot advance");
            frontier[middle] = true;
            for (int ti : mesh.faces[bf].tetras)
            {
                if (ti < 0)
                    throw std::runtime_error("Boundary barrier face");
                pending.push_back(ti);
                flags[ti] = true;
            }
        }
        while (!pending.empty())
        {
            int ti = pending.back();
            pending.pop_back();
            if (!flags[ti])
                continue;
            flags[ti] = false;
            std::vector<int> repaired, outputFaces;
            dfs(ti, repaired, outputFaces, true);
            append(repaired);
        }
    }
    if (std::find(owner.begin(), owner.end(), -1) != owner.end())
        throw std::runtime_error("Reference repair leaves tetrahedra uncovered");
    return owner;
}
#endif
