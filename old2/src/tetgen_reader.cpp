//
// Created by vigb9 on 4/5/2025.
//
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>

#include "gpolylla.h"

namespace gpolylla {
using namespace std;

int loadNode(Mesh* mesh, const string& filename) {
  ifstream nodeFile(filename);
  if (!nodeFile.is_open()) {
    cerr << "Unable to open file: " << filename << endl;
    return 0;
  }
  string line;
  getline(nodeFile, line);
  stringstream in(line);
  int numNodes = 0;
  in >> numNodes;
  mesh->vertices.reserve(numNodes);
  while (getline(nodeFile, line)) {
    in.str(line);
    string token;
    in >> token;
    if (token == "#") continue;
    float x, y, z;
    in >> x >> y >> z;
    mesh->vertices.emplace_back(x, y, z);
    in.clear();
  }

  return 1;
}

int loadEle(Mesh* mesh, const string& filename) {
  ifstream eleFile(filename);
  if (!eleFile.is_open()) {
    cerr << "Unable to open file: " << filename << endl;
    return 0;
  }
  string line;
  getline(eleFile, line);
  stringstream in(line);
  int numTetras = 0;
  in >> numTetras;
  mesh->tetras.reserve(numTetras);
  while (getline(eleFile, line)) {
    in.str(line);
    string token;
    in >> token;
    if (token == "#") continue;
    array verts = {-1, -1, -1, -1};
    in >> verts[0] >> verts[1] >> verts[2] >> verts[3];

    // sort
    ranges::sort(verts);
    mesh->tetras.emplace_back(verts);
    in.clear();
  }

  return 1;
}

int buildFaces(Mesh* mesh) {
  for (int ti = 0; ti < mesh->tetras.size(); ti++) {
    auto& tetra = mesh->tetras.at(ti);
    for (int i = 0; i < tetra.vertices.size(); i++) {
      array v = {tetra.vertices[(i + 1) % 4], tetra.vertices[(i + 2) % 4],
                 tetra.vertices[(i + 3) % 4]};
      if (!isOutside(mesh->vertices.at(v[0]), mesh->vertices.at(v[1]),
                     mesh->vertices.at(v[2]),
                     mesh->vertices.at(tetra.vertices[i]))) {
        v = {tetra.vertices[(i + 3) % 4], tetra.vertices[(i + 2) % 4],
             tetra.vertices[(i + 1) % 4]};
      }

      Face f(v, ti);
      tetra.faces[i] = mesh->faces.size();
      mesh->faces.push_back(f);
    }
  }

  return 1;
}

Mesh TetgenReader::operator()(const std::string& filename) {
  int ok = 1;
  Mesh mesh;
  cout << "Constructing mesh from files...\n";
  cout << "Reading vertex file (.node)...\n";
  ok &= loadNode(&mesh, filename + ".node");
  cout << "Reading tetrahedron file (.ele)...\n";
  ok &= loadEle(&mesh, filename + ".ele");

  if (!ok) {
    cerr << ".ele and .node files are mandatory\n";
    return {};
  }

  cout << "Building faces...\n";
  ok &= buildFaces(&mesh);

  if (!ok) {
    cerr << "Error building faces\n";
    return {};
  }

  cout << "Done!\n";
  return mesh;
};

}  // namespace gpolylla