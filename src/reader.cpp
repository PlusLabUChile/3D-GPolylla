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

vector<Vertex> readNode(const string &file)
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

vector<Tetrahedron> readEle(const string &file)
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

Mesh TetgenReader::readMesh()
{
    Mesh m;
    m.vertices = readNode(this->meshName + ".node");
    m.cells = readEle(this->meshName + ".ele");

    m.faces = buildFaces(m.vertices, m.cells);
    buildConnectivity(&m);
    return m;
}