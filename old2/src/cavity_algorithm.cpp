#include <algorithm>
#include <array>
#include <stdexcept>
#include <unordered_set>
#include <vector>

#include "gpolylla.h"

namespace gpolylla {
using std::array;
using std::move;
using std::unordered_set;
using std::vector;

// EXTRAS
class DepthFirstSearch {
 public:
  DepthFirstSearch(vector<int> *visited, CavityMesh *mesh,
                   CavityAlgorithm::Criteria *criteria)
      : visited(visited), criteria(criteria), mesh(mesh) {}

  void operator()(const int current, const int seed, unordered_set<int> *points,
                  vector<int> *faces, vector<int> *tetras) {
    visited->at(current) = seed;

    tetras->push_back(current);

    const CavityTetrahedron &currentTetra = mesh->tetras.at(current);

    for (int vi : currentTetra.vertices) {
      points->insert(vi);
    }

    for (int i = 0; i < 4; i++) {
      int nextTi = currentTetra.neighbours[i];
      if (nextTi == -1) {
        faces->push_back(currentTetra.faces[i]);
        continue;
      }

      if (visited->at(nextTi) != -1) {
        if (visited->at(nextTi) != seed)
          faces->push_back(currentTetra.faces[i]);
        continue;
      }

      if (criteria->isNext(seed, nextTi))
        (*this)(nextTi, seed, points, faces, tetras);
      else
        faces->push_back(currentTetra.faces[i]);

    }
  }

 private:
  vector<int> *visited;
  CavityAlgorithm::Criteria *criteria;
  CavityMesh *mesh;
};

// IMPLEMENTATION
PolyMesh CavityAlgorithm::operator()(const Mesh &m) {
  CavityMesh mesh(m);
  criteria->bind(&mesh);

  vector visited(mesh.tetras.size(), -1);

  vector seeds = criteria->getSeeds();

  DepthFirstSearch dfs(&visited, &mesh, criteria);

  vector<Polyhedron> polys;
  for (int ti : seeds) {
    if (visited[ti] != -1) continue;

    vector<int> faces;
    vector<int> tetras;
    unordered_set<int> uniquePoints;
    dfs(ti, ti, &uniquePoints, &faces, &tetras);

    vector points(uniquePoints.begin(), uniquePoints.end());
    polys.emplace_back(points, faces, tetras);
  }

  // Perform operations on mesh to create polyMesh
  criteria->unbind();
  return {.cells = move(polys),
          .tetras = m.tetras,
          .faces = move(mesh.faces),
          .vertices = move(mesh.vertices)};
}

void SphereCriteria::bind(CavityMesh *m) {
  mesh = m;
  spheres = vector<Sphere>(mesh->tetras.size());
  for (int ti = 0; ti < mesh->tetras.size(); ti++) {
    spheres[ti] =
        Sphere(mesh->vertexOfTetra(ti, 0), mesh->vertexOfTetra(ti, 1),
               mesh->vertexOfTetra(ti, 2), mesh->vertexOfTetra(ti, 3));
  }
}

void SphereCriteria::unbind() {
  mesh = nullptr;
  spheres.clear();
}

bool SphereCriteria::isNext(int current, int next) const {
  return spheres[next].isInside(spheres[current].center);
}

vector<int> SphereCriteria::getSeeds() const {
  vector<int> seeds(mesh->tetras.size());
  for (int ti = 0; ti < mesh->tetras.size(); ti++) {
    seeds[ti] = ti;
  }

  std::ranges::sort(seeds, [&](int i, int j) {
    return spheres[i].radius < spheres[j].radius;
  });

  return seeds;
}

}  // namespace gpolylla