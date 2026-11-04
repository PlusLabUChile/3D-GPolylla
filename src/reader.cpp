#include <cmath>
#include <fstream>
#include <set>
#include <sstream>
#include <unordered_set>

#include "logger.h"
#include "utils.h"

using namespace Polylla;
using namespace std;

class FileNotFoundError : public std::runtime_error
{
    using std::runtime_error::runtime_error;
};

namespace
{
std::string dataLine(std::istream &stream, const std::string &file)
{
    std::string line;
    while (std::getline(stream, line))
    {
        line = line.substr(0, line.find('#'));
        if (line.find_first_not_of(" \t\r\n") != std::string::npos)
            return line;
    }
    throw std::runtime_error("Truncated TetGen file: " + file);
}
} // namespace

vector<Face> buildFaces(const vector<Vertex> &vertices, const vector<Tetrahedron> &tetrahedrons)
{
    vector<Face> faces;
    unordered_set<Face> faceSet;

    for (int ti = 0; ti < tetrahedrons.size(); ++ti)
    {
        const Tetrahedron &t = tetrahedrons[ti];
        for (int i = 0; i < 4; ++i)
        {
            const int *config = FACE_CONFIGURATION[i];
            int id0 = t.vertices[config[0]];
            int id1 = t.vertices[config[1]];
            int id2 = t.vertices[config[2]];
            Face f(id0, id1, id2);
            faceSet.insert(f);
        }
    }

    for (const Face &f : faceSet)
    {
        faces.push_back(f);
    }

    // sort for consistent
    ranges::sort(faces, [](const Face &a, const Face &b) {
        auto sorted_a = a.vertices;
        auto sorted_b = b.vertices;
        ranges::sort(sorted_a);
        ranges::sort(sorted_b);
        return sorted_a < sorted_b;
    });
    return faces;
}

void buildConnectivity(Mesh *mesh)
{
    using facePos = std::pair<int, int>;
    unordered_map<Face, vector<facePos>> faceMap;

    for (int ti = 0; ti < mesh->tetras.size(); ++ti)
    {
        const Tetrahedron &t = mesh->tetras[ti];
        for (int i = 0; i < 4; ++i)
        {
            const int *config = FACE_CONFIGURATION[i];
            Face f(t.vertices[config[0]], t.vertices[config[1]], t.vertices[config[2]]);
            faceMap[f].emplace_back(ti, i);
        }
    }

    for (int fi = 0; fi < mesh->faces.size(); ++fi)
    {
        Face &f = mesh->faces[fi];
        if (!faceMap.contains(f))
            continue;

        const auto &positions = faceMap[f];
        if (positions.size() > 2)
            throw std::runtime_error("Non-manifold face");
        int i = 0;
        for (const auto &[ti, j] : positions)
        {
            f.tetras[i] = ti;
            mesh->tetras[ti].faces[j] = fi; // Associate face with tetrahedron
            i++;
        }
    }
}
// vector<Face> Polylla::makeDirectedFaces(const vector<Tetrahedron>& cells,
//                                         const vector<Vertex>& vertices) {
//   vector<Face> faces;
//   int cases[][] = {
//       {1, 2, 3},  // 0
//       {0, 2, 3},  // 1
//       {0, 1, 3},  // 2
//       {0, 1, 2}   // 3
//   };
//
//   for (const auto& c : cells) {
//     for (int vi = 0; vi < 4; ++vi) {
//       const int* config = cases[vi];
//       Vertex v0 = vertices[c.vertices[config[0]]];
//       Vertex v1 = vertices[c.vertices[config[1]]];
//       Vertex v2 = vertices[c.vertices[config[2]]];
//       Vertex other = vertices[c.vertices[vi]];
//       if (isOutside(v0, v1, v2, other)) {
//         faces.emplace_back(v0, v1, v2)
//       }
//     }
//   }
//
//   return faces;
// }

Mesh TetgenReader::readMesh()
{
    Mesh m;
    std::ifstream nodes(nodeFile), elements(eleFile);
    if (!nodes)
        throw std::runtime_error("Cannot open node file: " + nodeFile);
    if (!elements)
        throw std::runtime_error("Cannot open element file: " + eleFile);
    int count, dimension, attributes, markers;
    std::istringstream header(dataLine(nodes, nodeFile));
    if (!(header >> count >> dimension >> attributes >> markers) || count < 4 || dimension != 3 || attributes < 0 ||
        markers < 0)
        throw std::runtime_error("Invalid node header: " + nodeFile);
    std::unordered_map<int, int> indices;
    for (int i = 0; i < count; ++i)
    {
        std::istringstream row(dataLine(nodes, nodeFile));
        int id;
        double x, y, z, extra;
        if (!(row >> id >> x >> y >> z) || !std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z) ||
            indices.contains(id))
            throw std::runtime_error("Invalid or duplicate vertex in " + nodeFile);
        for (int j = 0; j < attributes + markers; ++j)
            if (!(row >> extra))
                throw std::runtime_error("Missing vertex attribute in " + nodeFile);
        indices[id] = i;
        m.vertices.emplace_back(x, y, z);
    }
    int corners;
    std::istringstream eleHeader(dataLine(elements, eleFile));
    if (!(eleHeader >> count >> corners >> attributes) || count < 1 || corners != 4 || attributes < 0)
        throw std::runtime_error("Invalid element header: " + eleFile);
    std::unordered_set<int> ids;
    std::set<std::array<int, 4>> uniqueTetras;
    for (int i = 0; i < count; ++i)
    {
        std::istringstream row(dataLine(elements, eleFile));
        int id;
        std::array<int, 4> verts;
        double extra;
        if (!(row >> id) || !ids.insert(id).second)
            throw std::runtime_error("Invalid element ID");
        for (int &v : verts)
        {
            int external;
            if (!(row >> external) || !indices.contains(external))
                throw std::runtime_error("Invalid vertex reference");
            v = indices.at(external);
        }
        for (int j = 0; j < attributes; ++j)
            if (!(row >> extra))
                throw std::runtime_error("Missing element attribute");
        auto key = verts;
        std::sort(key.begin(), key.end());
        if (std::adjacent_find(key.begin(), key.end()) != key.end() || !uniqueTetras.insert(key).second)
            throw std::runtime_error("Repeated vertex or duplicate tetrahedron");
        double volume6 =
            signedSixthVolume(m.vertices[verts[0]], m.vertices[verts[1]], m.vertices[verts[2]], m.vertices[verts[3]]);
        if (!std::isfinite(volume6) || volume6 == 0)
            throw std::runtime_error("Degenerate tetrahedron: " + std::to_string(i));
        m.tetras.emplace_back(verts);
    }
    m.faces = buildFaces(m.vertices, m.tetras);
    buildConnectivity(&m);
    return m;
}
