#include <gpolylla/algo.h>
#include <gpolylla/mesh.h>
#include <gpolylla/utils.h>

#include <unordered_set>
#include <vector>

namespace gpolylla {
    using Eigen::Vector3d;
    using std::unordered_set;
    using std::vector;
    using std::set;

    CavityAlgorithm::CavityAlgorithm(CavityAlgorithm::Criteria c) : c(c) {}

// First implementation
    PolyMesh CavityAlgorithm::operator()(const TetraMesh &m) {
        PolyMesh ans;
        mesh = CavityTetraMesh(m);
        calculateFittest();
        auto seeds = getSeeds();
        visited = vector<bool>(mesh.tetras.size(), false);

        for (int ti: seeds) {
            if (visited[ti]) continue;
            vector<int> faces;
            set<int> uniquePoints;
            depthFirstSearch(ti, &faces, &uniquePoints, ti);

            // construir el poliedro
            // revisar pq no respeta el orden ccw o cw
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
    }

    void CavityAlgorithm::depthFirstSearch(int ti, vector<int> *faces, set<int> *points, int seed) {
        visited[ti] = true;

        for (int vi: mesh.tetras[ti].vertices) {
            points->insert(vi);
        }

        for (int i = 0; i < mesh.tetras[ti].neighs.size(); i++) {
            int nextTi = mesh.tetras[ti].neighs[i];
            if (nextTi == -1) {
                faces->push_back(mesh.tetras[ti].faces[i]);
                continue;
            }

            if (visited[nextTi]) {
                faces->push_back(mesh.tetras[ti].faces[i]);
                continue;
            }

            if (fittests[nextTi].isIn(fittests[seed].center))
                depthFirstSearch(nextTi, faces, points, seed);
            else
                faces->push_back(mesh.tetras[ti].faces[i]);
        }
    }

    void CavityAlgorithm::calculateFittest() {
        fittests = std::vector<Sphere>(mesh.tetras.size());
        for (int ti = 0; ti < mesh.tetras.size(); ti++) {
            const auto &tetra = mesh.tetras[ti];
            fittests[ti] =
                    circumsphere(mesh.vertices[tetra[0]], mesh.vertices[tetra[1]],
                                 mesh.vertices[tetra[2]], mesh.vertices[tetra[3]]);
        }
    }

    vector<int> CavityAlgorithm::getSeeds() {
        vector<int> seeds(mesh.tetras.size());
        for (int ti = 0; ti < mesh.tetras.size(); ti++) {
            seeds[ti] = ti;
        }

        std::sort(seeds.begin(), seeds.end(), [&](int i, int j) {
            return c.value(i, *this) < c.value(j, *this);
        });
        return seeds;
    }

    double CavityAlgorithm::Criteria::value(int ti, const CavityAlgorithm &algo) {
        return algo.fittests[ti].radius;
    }

}  // namespace gpolylla
