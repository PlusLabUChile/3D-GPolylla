#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <ranges>

#include "polylla.h"

using namespace Polylla;
using namespace Eigen;
using namespace std;

Vector3i key(const Face& f) {}

int loadNode(const string& filename, ArrayX<Vertex>* vertices) {
  ifstream file(filename);
  if (!file.is_open()) {
    cerr << "Unable to open file: " << filename << endl;
    return 0;
  }
  string line;
  getline(file, line);
  stringstream in(line);
  int n = 0;
  in >> n;
  ArrayX<Vertex> temp(n);
  int idx = 0;
  while (idx < n) {
    getline(file, line);
    in.str(line);
    string token;
    in >> token;
    if (token == "#") continue;
    float x, y, z;
    in >> x >> y >> z;
    temp[idx] = Vertex(x, y, z);
    in.clear();
    idx++;
  }
  *vertices = temp;
  return 1;
}

int loadEle(const string& filename, ArrayX<Tetrahedron>* tetras) {
  ifstream file(filename);
  if (!file.is_open()) {
    cerr << "Unable to open file: " << filename << endl;
    return 0;
  }
  string line;
  getline(file, line);
  stringstream in(line);
  int n = 0;
  in >> n;
  ArrayX<Tetrahedron> temp(n);
  int idx = 0;
  while (idx < n) {
    getline(file, line);
    in.str(line);
    string token;
    in >> token;
    if (token == "#") continue;
    int v1, v2, v3, v4;
    in >> v1 >> v2 >> v3 >> v4;
    temp[idx] = Tetrahedron(v1, v2, v3, v4);
    in.clear();
    idx++;
  }
  *tetras = temp;
  return 1;
}

int faceBuilder(const ArrayX<Vertex>& vertices,
                const ArrayX<Tetrahedron>& tetras, ArrayX<Face>* faces) {
  // for each tetrahedron, create 4 faces if they do not exist
  map<Vector3i, Face> faceMap;
  for (const auto& t : tetras) {
    for (int i = 0; i < 4; i++) {
      Vector3i key(t[i], t[(i + 1) % 4], t[(i + 2) % 4]);
      // Ensure the key is ordered to avoid duplicates
      ranges::sort(key);
      // Check if the face already exists
      if (!faceMap.contains(key)) {
        // If it does not exist, add it to the map
        faceMap[key] = Face(key[0], key[1], key[2]);
      }
    }
  }

  // Convert the map to an ArrayX<Face>
  ArrayX<Face> temp(faceMap.size());
  int idx = 0;
  for (const auto& face : faceMap | views::values) {
    temp[idx++] = face;
  }

  return 1;
}

Mesh TetgenReader::read(const std::string& basename) {
  int ok = 1;
  ArrayX<Vertex> vertices;
  ArrayX<Tetrahedron> tetras;
  ArrayX<Face> faces;
  cout << "Constructing mesh from files...\n";
  cout << "Reading vertex file (" << basename << ".node)...\n";
  ok &= loadNode(basename + ".node", &vertices);
  cout << "Reading tetrahedron file (" << basename << ".ele)...\n";
  ok &= loadEle(basename + ".ele", &tetras);
  if (!ok) {
    cerr << ".ele and .node files are mandatory\n";
    return {};
  }
  ok &= faceBuilder(vertices, tetras, &faces);
  if (!ok) {
    cerr << "Error building faces from tetras\n";
    return {};
  }
  cout << "Done!\n";
  return {vertices, faces, tetras};
}
