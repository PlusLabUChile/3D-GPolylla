#include <gpolylla/mesh.h>

#include <unordered_map>

namespace gpolylla {
    using std::unordered_map;
    using std::vector;
    using std::ostream;


    CavityTetrahedron::CavityTetrahedron(const Tetrahedron &t)
            : Tetrahedron(t), neighs({-1, -1, -1, -1}) {}

    CavityTetraMesh::CavityTetraMesh(const TetraMesh &m) : vertices(m.vertices), faces(m.faces) {
        tetras.reserve(m.tetras.size());

        FaceTetraMesh aux(m);
//  for (const auto& t : aux.tetras) {
        for (int ti = 0; ti < aux.tetras.size(); ++ti) {
            const auto &t = aux.tetras[ti];
            CavityTetrahedron newTetra = m.tetras[ti];
            for (int i = 0; i < t.faces.size(); ++i) {
                const auto &f = aux.faces[t.faces[i]];
                if (f.twin == -1) continue;
                newTetra.neighs[i] = m.faces[f.twin].tetra;
            }

            tetras.push_back(newTetra);
        }
    }

    ostream &operator<<(ostream &out, const FaceTetraMesh &m) {
        out << "FACEMESH\n"
            << "    Total vertices: " << m.vertices.size() << "\n"
            << "    Total edges: " << m.vertices.size() << "\n"
            << "    Total faces: " << m.vertices.size() << "\n"
            << "    Total tetrahedrons: " << m.vertices.size() << "\n";
        out << "VERTICES\n";
        for (const auto &v: m.vertices) {
            out << "    " << v << "\n";
        }
        out << "FACES\n";
        for (const auto &f: m.faces) {
            out << "    " << f << "\n";
        }
        out << "TETRAHEDRONS\n";
        for (const auto &t: m.tetras) {
            out << "    " << t << "\n";
        }
        return out;
    }
}  // namespace gpolylla
