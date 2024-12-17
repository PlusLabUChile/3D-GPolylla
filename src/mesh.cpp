#include <gpolylla/mesh.h>

namespace gpolylla {
    using Eigen::Vector3d;
    using std::ostream;
    using std::vector;

    Vertex::Vertex() : Vertex(0.f, 0.f, 0.f) {}

    Vertex::Vertex(const Vertex &v) : Vertex(v.x, v.y, v.z) {}

    Vertex::Vertex(float x, float y, float z) : x(x), y(y), z(z) {}

    Vertex::operator Vector3d() const { return {x, y, z}; }

    bool Vertex::operator==(const Vertex &o) const {
        return x == o.x && y == o.y && z == o.z;
    }

    std::size_t Vertex::Hash::operator()(const Vertex &v) const {
        std::size_t h1 = std::hash<float>()(v.x);
        std::size_t h2 = std::hash<float>()(v.y);
        std::size_t h3 = std::hash<float>()(v.z);
        return h1 ^ (h2 << 1) ^ (h3 << 2);
    }

    ostream &operator<<(ostream &out, const Vertex &v) {
        out << "(Vertex " << v.x << " " << v.y << " " << v.z << ")";
        return out;
    }

    Edge::Edge() : Edge(-1, -1) {}

    Edge::Edge(const Edge &e) : Edge(e[0], e[1]) {}

    Edge::Edge(int v0, int v1) : vertices({v0, v1}) {}

    bool Edge::operator==(const Edge &e) const {
        return vertices[0] == e.vertices[0] && vertices[1] == e.vertices[1];
    }

// std::size_t Edge::Hash::operator()(const Edge& e) const {
////  return std::hash<int>()(e.idx);
//}

    std::size_t Edge::Hash::operator()(const Edge &e) const {
        std::size_t h1 = std::hash<int>()(e[0]);
        std::size_t h2 = std::hash<int>()(e[1]);
        return h1 ^ (h2 << 1);
    }

    ostream &operator<<(ostream &out, const Edge &e) {
        out << "(Edge ";
        for (auto &vi: e.vertices) out << " " << vi;
        out << ")";
        return out;
    }

    Face::Face() : Face(-1, -1, -1) {}

    Face::Face(const Face &f) : vertices(f.vertices), tetra(f.tetra) {}

    Face::Face(int v0, int v1, int v2) : vertices({v0, v1, v2}), tetra(-1) {}

    bool Face::operator==(const Face &f) const {
        return vertices[0] == f.vertices[0] && vertices[1] == f.vertices[1] &&
               vertices[2] == f.vertices[2];
    }

    std::size_t Face::Hash::operator()(const Face &f) const {
        std::size_t seed = 0;
        std::hash<int> hasher;
        for (int vertex: f.vertices) {
            seed ^= hasher(vertex) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
    }

    ostream &operator<<(ostream &out, const Face &f) {
        out << "(Face ";
        for (auto &vi: f.vertices) out << " " << vi;
        out << ")";
        return out;
    }

    Tetrahedron::Tetrahedron(int v0, int v1, int v2, int v3)
            : vertices({v0, v1, v2, v3}), faces() {}

    Tetrahedron::Tetrahedron() : Tetrahedron(-1, -1, -1, -1) {}

    Tetrahedron::Tetrahedron(const Tetrahedron &t) = default;

    bool Tetrahedron::operator==(const Tetrahedron &o) const {
        return vertices[0] == o.vertices[0] && vertices[1] == o.vertices[1] &&
               vertices[2] == o.vertices[2] && vertices[3] == o.vertices[3];
    }

    std::size_t Tetrahedron::Hash::operator()(const Tetrahedron &t) const {
        std::size_t seed = 0;
        std::hash<int> hasher;
        for (int vertex: t.vertices) {
            seed ^= hasher(vertex) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
    }

    ostream &operator<<(ostream &out, const Tetrahedron &t) {
        out << "(Tetrahedron ";
        for (auto &vi: t.vertices) out << " " << vi;
        out << ")";
        return out;
    }

    ostream &operator<<(ostream &out, const Polyhedron &p) {
        out << "(Polyhedron ";
        for (auto &vi: p.vertices) out << " " << vi;
//        out << ", tetras:";
//        for (auto &ti: p.tetras) out << " " << ti;
        out << ")";
        return out;
    }

    Polyhedron::Polyhedron(const vector<int> &vertices) : vertices(vertices) {}

    Polyhedron::Polyhedron(const gpolylla::Polyhedron &p) = default;

    bool Polyhedron::operator==(const Polyhedron &p) const {
        for (int i = 0; i < vertices.size(); i++) {
            if (vertices[i] != p.vertices[i]) return false;
        }
        return true;
    }

    std::size_t Polyhedron::Hash::operator()(const Polyhedron &p) const {
        std::size_t seed = 0;
        std::hash<int> hasher;
        for (int vertex: p.vertices) {
            seed ^= hasher(vertex) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
//        for (int tetra: p.faces) {
//            seed ^= hasher(tetra) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
//        }
        return seed;
    }

    ostream &operator<<(ostream &out, const TetraMesh &m) {
        out << "MESH\n"
            << "    Total vertices: " << m.vertices.size() << "\n"
            << "    Total edges: " << m.vertices.size() << "\n"
            << "    Total faces: " << m.vertices.size() << "\n"
            << "    Total tetrahedrons: " << m.vertices.size() << "\n";
        out << "VERTICES\n";
        for (const auto &v: m.vertices) {
            out << "    " << v << "\n";
        }
        out << "EDGES\n";
        for (const auto &e: m.edges) {
            out << "    " << e << "\n";
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

    ostream &operator<<(ostream &out, const PolyMesh &m) {
        out << "POLYMESH\n"
            << "    Total vertices: " << m.vertices.size() << "\n"
            << "    Total edges: " << m.vertices.size() << "\n"
            << "    Total faces: " << m.vertices.size() << "\n"
            << "    Total tetrahedrons: " << m.vertices.size() << "\n";
        out << "VERTICES\n";
        for (const auto &v: m.vertices) {
            out << "    " << v << "\n";
        }
        out << "EDGES\n";
        for (const auto &e: m.edges) {
            out << "    " << e << "\n";
        }
        out << "FACES\n";
        for (const auto &f: m.faces) {
            out << "    " << f << "\n";
        }
        out << "POLIHEDRAS\n";
        for (const auto &p: m.cells) {
            out << "    " << p << "\n";
        }
        return out;
    }

}  // namespace gpolylla
