#include <gpolylla/algo.h>
#include <gpolylla/mesh.h>
#include <gpolylla/utils.h>

#include <Eigen/Dense>
#include <cstdlib>
#include <iostream>
#include <unordered_set>
#include <vector>

namespace gpolylla {
    using Eigen::Vector3d;
    using std::unordered_set;
    using std::vector;
    using std::set;

    PolyMesh FaceAlgorithm::operator()(const TetraMesh &m) {
        PolyMesh ans;
        mesh = FaceTetraMesh(m);
        calculateFittest(m.faces);
        auto seeds = getSeeds();
        visited = std::vector<bool>(mesh.tetras.size(), false);

        for (int ti: seeds) {
            set<int> uniquePoints;
            vector<int> faces;
            depthFirstSearch(ti, &faces, &uniquePoints);

            // construir el poliedro
            vector<int> points(uniquePoints.begin(), uniquePoints.end());

            Polyhedron poly(points);
            poly.faces = faces;
            ans.cells.push_back(poly);
        }

        ans.vertices = m.vertices;
        ans.faces = m.faces;
        ans.edges = m.edges;
        ans.tetras = m.tetras;
        return ans;
    };

    void FaceAlgorithm::calculateFittest(const vector<Face> &faces) {
        for (int ti = 0; ti < mesh.tetras.size(); ++ti) {
            auto &t = mesh.tetras[ti];
            int fittest = -1;
            double max = 0.f;
            for (auto &fi: t.faces) {
                double temp = c.value(fi, *this);
                if (fittest == -1 || temp > max) {
                    fittest = fi;
                    max = temp;
                }
            }
            fittests[ti] = fittest;
        }
    }

    vector<int> FaceAlgorithm::getSeeds() {
        std::vector<int> seedFace;
        for (int fi = 0; fi < mesh.faces.size(); ++fi) {
            const auto &face = mesh.faces[fi];

//            if (face.tetra == -1) {
//                std::cerr << "POLYLLAFACE: Face " << f << " has bad tetras ("
//                          << face.tetras[0] << " & " << face.tetras[1] << ")";
//                exit(0);
//            }

            // If t1 is -1, check if f is the fittest of t0
            if (face.twin == -1) {
                if (fittests[face.tetra] == fi) {
                    seedFace.push_back(face.tetra);
                }
                continue;
            }
            // If  not -1, check if f its the fittest on both tetras
            int f0 = fittests[face.tetra];
            int f1 = fittests[mesh.faces[face.twin].tetra];
            if (fi == f0 && fi == f1) {
                seedFace.push_back(face.tetra);
            }
        }
        return seedFace;
    }

    void FaceAlgorithm::depthFirstSearch(int ti, vector<int> *faces, set<int> *points) {
        // int ti = mesh.faces[fi].tetras[0];
        visited[ti] = true;
        for (int vi: mesh.tetras[ti].vertices) {
            points->insert(vi);
        }

        for (int fi: mesh.tetras[ti].faces) {
            const auto &face = mesh.faces[fi];
            if (face.twin == -1) continue;
            int nextTi = mesh.faces[face.twin].tetra;
            if (visited[nextTi]) continue;
//            for (int temp: mesh.faces[fi].tetras) {
//                if (temp == ti) continue;
//                next = temp;
//            }

            if (fittests[nextTi] == fi)
                depthFirstSearch(nextTi, faces, points);
            else
                faces->push_back(fi);
        }
    }

    double FaceAlgorithm::Criteria::value(int fi, const FaceAlgorithm &algo) {
//        auto &tri = mesh.faces[fi];
//        Vector3d v0 = mesh.vertices[tri.vertices[0]];
//        Vector3d v1 = mesh.vertices[tri.vertices[1]];
//        Vector3d v2 = mesh.vertices[tri.vertices[2]];
        const auto &f = algo.mesh.faces[fi];
        return area(algo.mesh.vertices[f[0]], algo.mesh.vertices[f[1]], algo.mesh.vertices[f[2]]);
    }

}  // namespace gpolylla
