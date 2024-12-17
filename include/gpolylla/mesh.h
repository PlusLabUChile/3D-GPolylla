#ifndef _GPOLYLLA_MESH_H_
#define _GPOLYLLA_MESH_H_

#include <gpolylla/utils.h>

#include <Eigen/Dense>
#include <array>
#include <iostream>
#include <vector>

namespace gpolylla {
    class Vertex {
    public:
        float x, y, z;
        int tetra;

        Vertex();

        Vertex(float x, float y, float z);

        Vertex(const Vertex &v);

        // bool operator<(const Vertex&) const;
        bool operator==(const Vertex &) const;

        operator Eigen::Vector3d() const;

        struct Hash {
            std::size_t operator()(const Vertex &v) const;
        };

        friend std::ostream &operator<<(std::ostream &, const Vertex &);
    };

    class Edge {
    public:
        std::array<int, 2> vertices;
        int tetra;

        Edge();

        Edge(int v0, int v1);

        Edge(const Edge &e);

        bool operator==(const Edge &) const;

        inline std::size_t size() const { return vertices.size(); }

        inline int operator[](int idx) const { return vertices[idx]; }

        struct Hash {
            std::size_t operator()(const Edge &e) const;
        };

        friend std::ostream &operator<<(std::ostream &, const Edge &);
    };

    class Face {
    public:
        std::array<int, 3> vertices;
        int tetra;

        Face();

        Face(int v0, int v1, int v2);

        Face(const Face &f);

        bool operator==(const Face &) const;

        inline std::size_t size() const { return vertices.size(); }

        inline int operator[](int idx) const { return vertices[idx]; }

        struct Hash {
            std::size_t operator()(const Face &f) const;
        };

        friend std::ostream &operator<<(std::ostream &, const Face &);
    };

    class Tetrahedron {
    public:
        std::array<int, 4> vertices;
        std::array<int, 4> faces;

        Tetrahedron();

        Tetrahedron(int v0, int v1, int v2, int v3);

        Tetrahedron(const Tetrahedron &t);

        bool operator==(const Tetrahedron &t) const;

        inline std::size_t size() const { return vertices.size(); }

        inline int operator[](int idx) const { return vertices[idx]; }

        struct Hash {
            std::size_t operator()(const Tetrahedron &t) const;
        };

        friend std::ostream &operator<<(std::ostream &out, const Tetrahedron &t);
    };

    class Polyhedron {
    public:
        std::vector<int> vertices;
//  std::vector<int> tetras;
        std::vector<int> faces;

        Polyhedron() = default;

        Polyhedron(const std::vector<int> &vertices);

        Polyhedron(const Polyhedron &p);
        // Polyhedron(int idx, const std::vector<int>& faces);

        bool operator==(const Polyhedron &p) const;

        struct Hash {
            std::size_t operator()(const Polyhedron &p) const;
        };

        friend std::ostream &operator<<(std::ostream &out, const Polyhedron &p);
    };

    class TetraMesh {
    public:
        std::vector<Vertex> vertices;
        std::vector<Edge> edges;
        std::vector<Face> faces;
        std::vector<Tetrahedron> tetras;

        friend std::ostream &operator<<(std::ostream &out, const TetraMesh &m);
    };

    class PolyMesh {
    public:
        std::vector<Vertex> vertices;
        std::vector<Edge> edges;
        std::vector<Face> faces;
        std::vector<Tetrahedron> tetras;
        std::vector<Polyhedron> cells;

        friend std::ostream &operator<<(std::ostream &out, const PolyMesh &m);
    };


    class FaceFace : public Face {
    public:
//        std::array<int, 2> tetras;
//        int fittest;
        int twin;

        FaceFace(const Face &);

        friend std::ostream &operator<<(std::ostream &, const FaceFace &);
    };


    class FaceTetraMesh {
    public:
        std::vector<Vertex> vertices;
        std::vector<FaceFace> faces;
        std::vector<Tetrahedron> tetras;

        FaceTetraMesh() = default;

        FaceTetraMesh(const TetraMesh &m);

        friend std::ostream &operator<<(std::ostream &out, const FaceTetraMesh &m);
    };


    class CavityTetrahedron : public Tetrahedron {
    public:
        CavityTetrahedron(const Tetrahedron &t);

        std::array<int, 4> neighs;
    };

    class CavityTetraMesh {
    public:
        std::vector<Vertex> vertices;
        std::vector<Face> faces;
        std::vector<CavityTetrahedron> tetras;

        CavityTetraMesh() = default;

        CavityTetraMesh(const TetraMesh &m);

        friend std::ostream &operator<<(std::ostream &out, const CavityTetraMesh &m);
    };

}  // namespace gpolylla
#endif  // _GPOLYLLA_MESH_BASIC_MESH_H
