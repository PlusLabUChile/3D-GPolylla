#include "mesh.h"

#include <fstream>
#include <iostream>
#include <sstream>

namespace GPolylla {
void TetrahedronMesh::construct_tetrahedral_mesh(
    const std::string &node_filename, const std::string &face_filename,
    const std::string &ele_filename, const std::string &edge_filename) {
  read_node_file(node_filename);
  read_face_file(face_filename);
  read_ele_file(ele_filename);
  read_edge_file(edge_filename);
}

void TetrahedronMesh::read_node_file(const std::string &filename) {
  std::cout << "Reading node file: " << filename << std::endl;
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
  nodes.reserve(num_nodes);
  std::cout << "Number of nodes: " << num_nodes << std::endl;
  while (std::getline(file, line)) {
    std::istringstream iss(line);
    std::string token;
    iss >> token;
    if (token == "#") continue;
    Vertex node;
    node.id = std::stoi(token);
    iss >> node.x >> node.y >> node.z;
    nodes.push_back(node);
  }
  file.close();
}

void TetrahedronMesh::read_face_file(const std::string &filename) {
  std::cout << "Reading face file: " << filename << std::endl;
  std::ifstream file(filename);
  if (!file.is_open()) {
    std::cerr << "Unable to open file: " << filename << std::endl;
    exit(1);
  }
  std::string line;
  std::getline(file, line);
  std::istringstream iss(line);
  int num_faces;
  iss >> num_faces;
  faces.reserve(num_faces);
  std::cout << "Number of faces: " << num_faces << std::endl;
  while (std::getline(file, line)) {
    std::istringstream iss(line);
    std::string token;
    iss >> token;
    if (token == "#") continue;
    Face face;
    face.id = std::stoi(token);
    iss >> face.v1 >> face.v2 >> face.v3;
    int bm;
    iss >> bm;
    face.is_boundary = (bm == 1 || bm == -1);
    iss >> face.n1 >> face.n2;
    faces.push_back(face);
  }
  file.close();
}

void TetrahedronMesh::read_ele_file(const std::string &filename) {
  std::cout << "Reading ele file: " << filename << std::endl;
  std::ifstream file(filename);
  if (!file.is_open()) {
    std::cerr << "Unable to open file: " << filename << std::endl;
    exit(1);
  }
  std::string line;
  std::getline(file, line);
  std::istringstream iss(line);
  int num_tetras;
  iss >> num_tetras;
  tetras.reserve(num_tetras);
  std::cout << "Number of tetrahedrons: " << num_tetras << std::endl;
  while (std::getline(file, line)) {
    std::istringstream iss(line);
    std::string token;
    iss >> token;
    if (token == "#") continue;
    Tetrahedron t;
    t.id = std::stoi(token);
    iss >> t.v1 >> t.v2 >> t.v3 >> t.v4;
    tetras.push_back(t);
  }
  file.close();
}

void TetrahedronMesh::read_edge_file(const std::string &filename) {
  std::cout << "Reading edge file: " << filename << std::endl;
  std::ifstream file(filename);
  if (!file.is_open()) {
    std::cerr << "Unable to open file: " << filename << std::endl;
    exit(1);
  }
  std::string line;
  std::getline(file, line);
  std::istringstream iss(line);
  int num_edges;
  iss >> num_edges;
  tetras.reserve(num_edges);
  std::cout << "Number of edges: " << num_edges << std::endl;
  while (std::getline(file, line)) {
    std::istringstream iss(line);
    std::string token;
    iss >> token;
    if (token == "#") continue;
    Edge e;
    e.id = std::stoi(token);
    iss >> e.v1 >> e.v2;
    int bm;
    iss >> bm;
    e.is_boundary = bm == 1;
    iss >> e.first_tetra;
    edges.push_back(e);
  }
  file.close();
}

TetrahedronMesh::TetrahedronMesh(const std::string &node_filename,
                                 const std::string &face_filename,
                                 const std::string &ele_filename,
                                 const std::string &edge_filename) {
  construct_tetrahedral_mesh(node_filename, face_filename, ele_filename,
                             edge_filename);
}

Face &TetrahedronMesh::get_face(int id) { return faces[id]; }
Tetrahedron &TetrahedronMesh::get_tetra(int id) { return tetras[id]; }
Edge &TetrahedronMesh::get_edge(int id) { return edges[id]; }
Vertex &TetrahedronMesh::get_vertex(int id) { return nodes[id]; }
int TetrahedronMesh::num_faces() { return faces.size(); }
int TetrahedronMesh::num_tetrahedrons() { return tetras.size(); }
int TetrahedronMesh::num_edges() { return edges.size(); }
int TetrahedronMesh::num_nodes() { return nodes.size(); }

void TetrahedronMesh::display_info() {
  std::cout << "Number of nodes: " << num_nodes() << std::endl;
  std::cout << "Number of faces: " << num_faces() << std::endl;
  std::cout << "Number of tetrahedrons: " << num_tetrahedrons() << std::endl;
  std::cout << "Number of edges: " << num_edges() << std::endl;
}

}  // namespace GPolylla
