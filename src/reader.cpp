#include <fstream>
#include <unordered_set>

#include "logger.h"
#include "utils.h"

using namespace Polylla;
using namespace std;

class FileNotFoundError : public std::runtime_error
{
    using std::runtime_error::runtime_error;
};

vector<Vertex> buildVertices(const string &file)
{
    vector<Vertex> verts;

    ifstream nodeStream(file);
    if (!nodeStream.is_open())
    {
        throw FileNotFoundError("Cannot open node file: " + file);
    }

    string line;
    getline(nodeStream, line);
    istringstream headerStream(line);
    int numVertices;
    headerStream >> numVertices;
    verts.reserve(numVertices);

    while (getline(nodeStream, line))
    {
        istringstream lineStream(line);
        string token;
        lineStream >> token;
        if (token == "#")
            continue; // Skip comments
        float x, y, z;
        lineStream >> x >> y >> z;
        verts.emplace_back(x, y, z);
    }
    nodeStream.close();
    return verts;
}

vector<Tetrahedron> buildCells(const string &file)
{
    vector<Tetrahedron> cells;
    ifstream eleStream(file);
    if (!eleStream.is_open())
    {
        throw FileNotFoundError("Cannot open element file: " + file);
    }

    string line;
    getline(eleStream, line);
    istringstream headerStream(line);
    int numCells;
    headerStream >> numCells;
    cells.reserve(numCells);
    while (getline(eleStream, line))
    {
        istringstream lineStream(line);
        string token;
        lineStream >> token;
        if (token == "#")
            continue; // Skip comments
        int v0, v1, v2, v3;
        lineStream >> v0 >> v1 >> v2 >> v3;
        cells.emplace_back(v0, v1, v2, v3);
    }
    eleStream.close();
    return cells;
}

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

    for (int ti = 0; ti < mesh->cells.size(); ++ti)
    {
        const Tetrahedron &t = mesh->cells[ti];
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
        int i = 0;
        for (const auto &[ti, j] : positions)
        {
            f.tetras[i] = ti;
            mesh->cells[ti].faces[j] = fi; // Associate face with tetrahedron
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
    m.vertices = buildVertices(this->meshName + ".node");
    m.cells = buildCells(this->meshName + ".ele");

    m.faces = buildFaces(m.vertices, m.cells);
    buildConnectivity(&m);
    return m;
}