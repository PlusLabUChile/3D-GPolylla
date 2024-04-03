#include "gpolylla/mesh.h"

#include <fstream>
#include <iostream>
#include <sstream>

namespace gpolylla {
using std::string;
using std::vector;

void get_vertices(const string &filename, vector<Vertex> *vertices) {
  std::ifstream file(filename);
  if (!file.is_open()) {
    std::cerr << "Unable to open file: " << filename << std::endl;
    exit(1);
  }
  std::string line;
  std::getline(file, line);
  std::istringstream iss(line);
  int num_nodes;
  iss >> num_nodes;
  vertices->reserve(num_nodes);
  while (std::getline(file, line)) {
    std::istringstream iss(line);
    std::string token;
    iss >> token;
    if (token == "#") continue;
    Vertex node;
    iss >> node.x >> node.y >> node.z;
    vertices->push_back(node);
  }
  file.close();
}

TetrahedronMesh::TetrahedronMesh(const string &node_filename,
                                 const string &face_filename,
                                 const string &ele_filename) {}

}  // namespace gpolylla
