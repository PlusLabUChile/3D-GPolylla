#include <fstream>
#include <iostream>

#include "polylla.h"

using namespace Polylla;
using namespace Eigen;
using namespace std;

ArrayX<Vertex> loadNode(string& filename) {
  ifstream file(filename);
  if (!file.is_open()) {
    cerr << "Unable to open file: " << filename << endl;
    return ArrayX<Vertex>();
  }
  string line;
  getline(file, line);
  stringstream in(line);
  int n = 0;
  in >> n;
  ArrayX<Vertex> vertices(n);
  int idx = 0;
  while (idx < n) {
    getline(file, line);
    in.str(line);
    string token;
    in >> token;
    if (token == "#") continue;
    float x, y, z;
    in >> x >> y >> z;
    vertices[idx] = Vertex(x, y, z);
    in.clear();
    idx++;
  }
  return vertices;
}
ArrayX<Tetrahedron> loadEle(string& filename) {
  ifstream file(filename);
  if (!file.is_open()) {
    cerr << "Unable to open file: " << filename << endl;
    return 0;
  }
  string line;
  getline(file, line);
  stringstream in(line);
  int numTetras = 0;
  in >> numTetras;
  while (numTetras) {
    getline(file, line);
    in.str(line);
    string token;
    in >> token;
    if (token == "#") continue;
    int v1, v2, v3, v4;
    in >> v1 >> v2 >> v3 >> v4;
    Tetrahedron t(v1, v2, v3, v4);
    *tetras << t;
    in.clear();
    numTetras--;
  }
  return 1;
}

Mesh TetgenReader::read(const std::string& basename) {
  int ok = 1;
  Mesh mesh;

  cout << "Constructing mesh from files...\n";
  cout << "Reading vertex file (" << basename << ".node)...\n";
  ok &= loadNode(&mesh, basename + ".node");
  cout << "Reading tetrahedron file (" << basename << ".ele)...\n";
  ok &= loadEle(&mesh, basename + ".ele");
  if (!ok) {
    cerr << ".ele and .node files are mandatory\n";
    return mesh;
  }
  cout << "Done!\n";
  return mesh;
}
