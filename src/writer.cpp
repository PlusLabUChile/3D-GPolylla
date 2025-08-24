#include "utils.h"
#include <fstream>


using namespace Polylla;
using namespace std;

struct DirectedInfo
{
    vector<array<int, 3>> faces;
    vector<vector<int>> cells;
};

bool isOutside(int fi, int vi, const PolyMesh &mesh)
{
    const Vertex &ref = mesh.vertices[vi];
    const Vertex &v0 = mesh.vertices[mesh.faces[fi].vertices[0]];
    const Vertex &v1 = mesh.vertices[mesh.faces[fi].vertices[1]];
    const Vertex &v2 = mesh.vertices[mesh.faces[fi].vertices[2]];
    const auto normal = (v1 - v0).cross(v2 - v0);
    const auto toRef = ref - v0;
    return normal.dot(toRef) > 0;
}

DirectedInfo getDirectedFaces(PolyMesh *mesh)
{
    DirectedInfo info;
    info.cells.resize(mesh->cells.size());
    int fi = 0;
    for (int pi = 0; pi < mesh->cells.size(); ++pi)
    {
        const Polyhedron &p = mesh->cells[pi];
        for (const int ti : p.cells)
        {
            const Tetrahedron &t = mesh->tetras[ti];
            for (const int tetraFi : t.faces)
            {
                if (ranges::find(p.faces, tetraFi) == p.faces.end())
                    continue;

                const Face &f = mesh->faces[tetraFi];
                for (const int ref : t.vertices)
                {
                    // Ref is the only vertex that is not part of the face
                    if (ranges::find(f.vertices, ref) != f.vertices.end())
                        continue;

                    // Direct the face based on the reference vertex
                    auto vertices = f.vertices;
                    if (!isOutside(tetraFi, ref, *mesh))
                    {
                        // If the reference vertex is inside, reverse the order
                        ranges::reverse(vertices);
                    }
                    info.faces.push_back(vertices);
                    info.cells[pi].push_back(fi);
                    fi++;
                    break;
                }
            }
        }
    }
    return info;
};

void VisFWriter::writeMesh(PolyMesh mesh)
{
    ofstream file(meshName + ".visf");
    if (!file.is_open())
    {
        throw runtime_error("Unable to create file: " + meshName + ".visf");
    }

    // primer valor -> formato del archivo
    // 0 -> big endian
    // 1 -> little endian
    // 2 -> ascii

    // segundo valor -> tipo de malla
    // 0 -> nube de puntos
    // 1 -> malla de poligonos
    // 2 -> malla de poliedros
    file << 2 << " " << 2 << endl;

    // cantidad de puntos y puntos
    file << mesh.vertices.size() << endl;
    for (const auto &v : mesh.vertices)
    {
        file << v.x() << " " << v.y() << " " << v.z() << endl;
    }

    // cantidad de poligonos y poligonos
    auto info = getDirectedFaces(&mesh);
    file << info.faces.size() << endl;
    for (const auto &f : mesh.faces)
    {
        file << f.vertices.size();
        for (int vi : f.vertices)
        {
            file << " " << vi;
        }
        file << endl;
    }

    // relacion de vecindad entre poligonos
    file << 0 << endl;
    // numero de poliedros y poliedros (basado en poligonos)
    file << mesh.cells.size() << endl;
    for (int pi = 0; pi < mesh.cells.size(); ++pi)
    {
        const auto &p = mesh.cells[pi];
        file << p.faces.size();
        for (int fi : p.faces)
        {
            file << " " << fi;
        }
        file << endl;
    }
}
