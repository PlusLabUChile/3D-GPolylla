#include <gpolylla.h>

#include <unordered_set>

namespace gpolylla {
using std::vector;
using std::unordered_set;

class DepthFirstSearch {
 public:
  DepthFirstSearch(vector<int> *visited, FaceMesh *mesh,
                   CavityAlgorithm::Criteria *criteria)
      : visited(visited), mesh(mesh), criteria(criteria) {}
  void operator()(const int current, const int seed, unordered_set<int> *points,
    vector<int> *faces, vector<int> *tetras) {
    visited->at(current) = seed;
    tetras->push_back(current);
    const auto &currentTetra = mesh->tetras.at(current);

    for (int vi : currentTetra.vertices) {
      points->insert(vi);
    }

    for (int i = 0; i < currentTetra.faces.size(); i++) {
      int fi = currentTetra.faces.at(i);
      const auto &face = mesh->faces.at(fi);
      if (face.twin == -1) {
        faces->push_back(fi);
        continue;
      }

      int nextTi = mesh->faces[face.twin].tetra;

      if (visited->at(nextTi) != -1) {
        if (visited->at(nextTi) != seed)
          faces->push_back(fi);
        continue;
      }

      // if (fittests[nextTi] == fi)
      if (criteria->isNext(seed, nextTi))
        (*this)(nextTi, seed, points, faces, tetras);
      else
        faces->push_back(fi);
    }
  }

 private:
  vector<int> *visited;
  FaceMesh *mesh;
  CavityAlgorithm::Criteria *criteria;
};

PolyMesh FaceAlgorithm::operator()(const Mesh &m) {
  FaceMesh mesh(m);
  criteria->bind(&mesh);

  vector visited(mesh.tetras.size(), -1);

  vector<int> seeds = criteria->getSeeds();

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

  // PolyMesh ans;
  // mesh = FaceTetraMesh(m);
  // calculateFittest(m.faces);
  // auto seeds = getSeeds();
  // visited = std::vector<bool>(mesh.tetras.size(), false);
  //
  // for (int ti : seeds) {
  //   set<int> uniquePoints;
  //   vector<int> faces;
  //   depthFirstSearch(ti, &faces, &uniquePoints);
  //
  //   // construir el poliedro
  //   vector<int> points(uniquePoints.begin(), uniquePoints.end());
  //
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
};

}  // namespace gpolylla