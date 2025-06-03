#include <gpolylla.h>

#include <unordered_set>

namespace gpolylla {
using std::unordered_set;
using std::vector;

class DepthFirstSearch {
 public:
  DepthFirstSearch(vector<int> *visited, FaceMesh *mesh,
                   FaceAlgorithm::Criteria *criteria)
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
        if (visited->at(nextTi) != seed) faces->push_back(fi);
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
  FaceAlgorithm::Criteria *criteria;
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

void AreaCriteria::bind(FaceMesh *m) {
  mesh = m;
  fittests = vector<int>(mesh->tetras.size());
  for (int ti = 0; ti < mesh->tetras.size(); ++ti) {
    auto &t = mesh->tetras.at(ti);
    int fittest = -1;
    double max = 0.f;
    for (auto &fi : t.faces) {
      const auto &face = mesh->faces.at(fi);
      double temp = area(mesh->vertices.at(face.vertices[0]),
                         mesh->vertices.at(face.vertices[1]),
                         mesh->vertices.at(face.vertices[2]));
      if (fittest == -1 || temp > max) {
        fittest = fi;
        max = temp;
      }
    }
    fittests[ti] = fittest;
  }
}

void AreaCriteria::unbind() {
  mesh = nullptr;
  fittests.clear();
}

bool AreaCriteria::isNext(int current, int next) {
  return fittests[next] == current;
}

vector<int> AreaCriteria::getSeeds() {
  vector<int> seedFace;
  for (int fi = 0; fi < mesh->faces.size(); ++fi) {
    const auto &face = mesh->faces[fi];

    // If t1 is -1, check if f is the fittest of t0
    if (face.twin == -1) {
      if (fittests[face.tetra] == fi) {
        seedFace.push_back(face.tetra);
      }
      continue;
    }
    // If  not -1, check if f its the fittest on both tetras
    int f0 = fittests[face.tetra];
    int f1 = fittests[mesh->faces[face.twin].tetra];
    if (fi == f0 && fi == f1) {
      seedFace.push_back(face.tetra);
    }
  }
  return seedFace;
}

}  // namespace gpolylla