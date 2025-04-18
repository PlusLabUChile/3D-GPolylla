#include <gpolylla/algo.h>
#include <gpolylla/mesh.h>
#include <gpolylla/utils.h>

#include <unordered_set>
#include <vector>

namespace gpolylla {
using Eigen::Vector3d;
using std::set;
using std::unordered_set;
using std::vector;

vector<Sphere> getSpheres(const CavityTetraMesh &mesh) {
  vector<Sphere> spheres = vector<Sphere>(mesh.tetras.size());
  for (int ti = 0; ti < mesh.tetras.size(); ti++) {
    const auto &tetra = mesh.tetras[ti];
    spheres[ti] =
        circumsphere(mesh.vertices[tetra[0]], mesh.vertices[tetra[1]],
                     mesh.vertices[tetra[2]], mesh.vertices[tetra[3]]);
  }
  return spheres;
}

CavityAlgorithm::CavityAlgorithm(CavityAlgorithm::Criteria c) : c(c) {}

// First implementation
PolyMesh CavityAlgorithm::operator()(const TetraMesh &m) override {
  CavityTetraMesh mesh(m);
  PolyMesh pMesh;

  vector<Sphere> spheres = getSpheres(mesh);
  vector<int> seeds = getSeeds();

  vector<Polyhedron> polys;
  visited = vector(mesh.tetras.size(), -1);

  pMesh.vertices = mesh.vertices;
  pMesh.faces = mesh.faces;
  pMesh.tetras = mesh.tetras;
  pMesh.cells = polys;

  return pMesh;

  // mesh = CavityTetraMesh(m);
  // calculateFittest();
  // auto seeds = getSeeds();
  // visited = vector<int>(mesh.tetras.size(), -1);
  //
  // for (int ti : seeds) {
  //   if (visited[ti] != -1) continue;
  //   vector<int> faces;
  //   set<int> uniquePoints;
  //   depthFirstSearch(ti, &faces, &uniquePoints, ti);
  //
  //   // construir el poliedro
  //   // revisar pq no respeta el orden ccw o cw
  //   vector<int> points(uniquePoints.begin(), uniquePoints.end());
  //   Polyhedron poly(points);
  //   poly.faces = faces;
  //   ans.cells.push_back(poly);
  // }
  //
  // ans.vertices = m.vertices;
  // ans.faces = m.faces;
  // ans.edges = m.edges;
  // ans.tetras = m.tetras;
  // return ans;
}

void CavityAlgorithm::constructSpheres() {}

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

vector<Polyhedron> CavityAlgorithm::constructPolys(vector<int> seeds) {
  visited = vector<int>(mesh.tetras.size(), -1);
  for (int ti : seeds) {
    if (visited[ti] != -1) continue;
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
}

void CavityAlgorithm::depthFirstSearch(int ti, vector<int> *faces,
                                       set<int> *points, int seed) {
  visited[ti] = seed;

  for (int vi : mesh.tetras[ti].vertices) {
    points->insert(vi);
  }

  for (int i = 0; i < mesh.tetras[ti].neighs.size(); i++) {
    int nextTi = mesh.tetras[ti].neighs[i];
    if (nextTi == -1) {
      faces->push_back(mesh.tetras[ti].faces[i]);
      continue;
    }

    if (visited[nextTi] != -1) {
      if (visited[nextTi] != seed) faces->push_back(mesh.tetras[ti].faces[i]);
      continue;
    }

    if (fittests[nextTi].isIn(fittests[seed].center))
      depthFirstSearch(nextTi, faces, points, seed);
    else
      faces->push_back(mesh.tetras[ti].faces[i]);
  }
}

// void CavityAlgorithm::calculateFittest() {
//   fittests = std::vector<Sphere>(mesh.tetras.size());
//   // for (int ti = 0; ti < mesh.tetras.size(); ti++) {
//   //   const auto &tetra = mesh.tetras[ti];
//   //   fittests[ti] =
//   //       circumsphere(mesh.vertices[tetra[0]], mesh.vertices[tetra[1]],
//   //                    mesh.vertices[tetra[2]], mesh.vertices[tetra[3]]);
//   // }
// }

// vector<int> CavityAlgorithm::getSeeds() {
//   vector<int> seeds(mesh.tetras.size());
//   for (int ti = 0; ti < mesh.tetras.size(); ti++) {
//     seeds[ti] = ti;
//   }
//
//   std::sort(seeds.begin(), seeds.end(), [&](int i, int j) {
//     return c.value(i, *this) < c.value(j, *this);
//   });
//   return seeds;
// }

double CavityAlgorithm::Criteria::value(int ti, const CavityAlgorithm &algo) {
  return algo.fittests[ti].radius;
}

AlgorithmStats CavityAlgorithm::stats() const {
  AlgorithmStats stats;
  calculatePolyhedronKernel(ans, &stats);
  return stats;
}

}  // namespace gpolylla
