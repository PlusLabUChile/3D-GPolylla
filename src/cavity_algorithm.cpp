#include <gpolylla/algo.h>
#include <gpolylla/mesh.h>
#include <gpolylla/utils.h>

#include <unordered_set>
#include <vector>

#include "Eigen/Core"

#define EPSILON 0.001

namespace gpolylla {
using Eigen::Vector3d;
using std::unordered_set;
using std::vector;

PolyMesh CavityAlgorithm::operator()(const TetraMesh& m) {
  PolyMesh ans;
  mesh = CavityTetraMesh(m);
  calculateFittest();
  auto seeds = getSeeds();
  visited = std::vector<bool>(mesh.tetras.size(), false);

  for (int ti : seeds) {
    if (visited[ti]) continue;
    std::vector<int> polyTetras;
    auto& currentTetra = mesh.tetras[ti];
    currentFittest = &currentTetra.fittest;
    Vector3d pointAsVector = mesh.vertices[currentTetra.vertices[0]];
    currentSquaredNorm = (*currentFittest - pointAsVector).squaredNorm();
    depthFirstSearch(ti, &polyTetras);

    // construir el poliedro
    unordered_set<int> uniVertex;

    for (int pti : polyTetras) {
      for (int vi : mesh.tetras[pti].vertices) {
        uniVertex.insert(vi);
      }
    }

    vector<int> polyVert;
    polyVert.reserve(uniVertex.size());

    for (int vi : uniVertex) {
      polyVert.push_back(vi);
    }

    Polyhedron poly(ans.cells.size(), polyVert);
    poly.tetras = polyTetras;  // cambiar
    ans.cells.push_back(poly);
  }

  ans.vertices = m.vertices;
  ans.faces = m.faces;
  ans.edges = m.edges;
  ans.tetras = m.tetras;
  return ans;
}

void CavityAlgorithm::depthFirstSearch(int ti, vector<int>* polyhedron) {
  // int ti = mesh.faces[fi].tetras[0];
  visited[ti] = true;
  polyhedron->push_back(ti);

  for (int nextTi : mesh.tetras[ti].neighs) {
    if (nextTi == -1) break;
    if (visited[nextTi]) continue;

    const auto& nextTetra = mesh.tetras[nextTi];
    bool isInside = true;
    for (int vi : nextTetra.vertices) {
      if (!isInside) break;
      Vector3d asVector = mesh.vertices[vi];
      double squaredDistance = (asVector - *currentFittest).squaredNorm();
      isInside = squaredDistance <= currentSquaredNorm + EPSILON;
    }

    // const auto& thisTetra = mesh.tetras[ti];
    if (isInside) depthFirstSearch(nextTi, polyhedron);
  }
}

void CavityAlgorithm::calculateFittest() {
  for (auto& t : mesh.tetras) {
    t.fittest = c.value(t.idx, mesh);
  }
}

vector<int> CavityAlgorithm::getSeeds() {
  vector<int> seeds;
  seeds.reserve(mesh.tetras.size());

  for (auto& t : mesh.tetras) {
    seeds.push_back(t.idx);
  }

  return seeds;
}

Eigen::Vector3d CavityAlgorithm::Criteria::value(int ti,
                                                 const CavityTetraMesh& mesh) {
  const auto& t = mesh.tetras[ti];
  return circumsphere(
      mesh.vertices[t.vertices[0]], mesh.vertices[t.vertices[1]],
      mesh.vertices[t.vertices[2]], mesh.vertices[t.vertices[3]]);
}

// vector<int> CavityAlgorithm::Criteria::value(int ti,
//                                              const CavityTetraMesh& mesh) {
//   auto& t = mesh.tetras[ti];
//   vector<int> fittest;
//   std::queue<int> tetras;
//
//   auto center =
//       circumsphere(mesh.vertices[t.vertices[0]],
//       mesh.vertices[t.vertices[1]],
//                    mesh.vertices[t.vertices[2]],
//                    mesh.vertices[t.vertices[3]]);
//   double sqRadius =
//       (center - Vector3d(mesh.vertices[t.vertices[0]])).squaredNorm();
//
//   tetras.push(ti);
//   while (!tetras.empty()) {
//     auto& currentIdx = tetras.front();
//     tetras.pop();
//     auto& current = mesh.tetras[currentIdx];
//     for (int nextIdx : current.neighs) {
//       if (nextIdx == -1) break;
//
//       if (std::find(fittest.begin(), fittest.end(), nextIdx) !=
//       fittest.end())
//         continue;
//
//       bool isIn = true;
//       for (int vi : mesh.tetras[nextIdx].vertices) {
//         Vector3d p = mesh.vertices[vi];
//         double sqDistance = (p - center).squaredNorm();
//         isIn = isIn && sqDistance <= sqRadius;
//       }
//
//       if (isIn) {
//         tetras.push(nextIdx);
//         fittest.push_back(nextIdx);
//       }
//     }
//   }
//   return fittest;
// }

}  // namespace gpolylla
