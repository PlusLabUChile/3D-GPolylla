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

PolyMesh FaceAlgorithm::operator()(const TetraMesh& m) {
  PolyMesh ans;
  mesh = FaceTetraMesh(m);
  calculateFittest(m.faces);
  auto seeds = getSeeds();
  visited = std::vector<bool>(mesh.tetras.size(), false);

  for (int ti : seeds) {
    std::vector<int> polyTetras;
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
};

void FaceAlgorithm::calculateFittest(const vector<Face>& faces) {
  for (auto& t : mesh.tetras) {
    int fittest = -1;
    double max = 0.f;
    for (auto& fi : t.faces) {
      double temp = c.value(fi, mesh);
      if (fittest == -1 || temp > max) {
        fittest = fi;
        max = temp;
      }
    }
    t.fittest = fittest;
  }
}

vector<int> FaceAlgorithm::getSeeds() {
  std::vector<int> seedFace;

  for (int f = 0; f < mesh.faces.size(); ++f) {
    const auto& face = mesh.faces[f];

    if (face.tetras[0] == -1) {
      std::cerr << "POLYLLAFACE: Face " << f << " has bad tetras ("
                << face.tetras[0] << " & " << face.tetras[1] << ")";
      exit(0);
    }

    // If t1 is -1, check if f is the fittest of t0
    if (face.tetras[1] == -1) {
      if (mesh.tetras[face.tetras[0]].fittest == f) {
        seedFace.push_back(face.tetras[0]);
      }
      continue;
    }
    // If  not -1, check if f its the fittest on both tetras
    int f0 = mesh.tetras[face.tetras[0]].fittest;
    int f1 = mesh.tetras[face.tetras[1]].fittest;
    if (f == f0 && f == f1) {
      seedFace.push_back(face.tetras[0]);
    }
  }
  return seedFace;
}

void FaceAlgorithm::depthFirstSearch(int ti, vector<int>* polyhedron) {
  // int ti = mesh.faces[fi].tetras[0];
  visited[ti] = true;
  polyhedron->push_back(ti);

  for (int fi : mesh.tetras[ti].faces) {
    int next = -1;

    for (int temp : mesh.faces[fi].tetras) {
      if (temp == ti) continue;
      next = temp;
    }

    if (next == -1) continue;

    if (mesh.tetras[next].fittest == fi && !visited[next])
      depthFirstSearch(next, polyhedron);
  }
}

double FaceAlgorithm::Criteria::value(int fi, const FaceTetraMesh& mesh) {
  auto& tri = mesh.faces[fi];
  Vector3d v0 = mesh.vertices[tri.vertices[0]];
  Vector3d v1 = mesh.vertices[tri.vertices[1]];
  Vector3d v2 = mesh.vertices[tri.vertices[2]];
  return area(v0, v1, v2);
}

}  // namespace gpolylla
