#include "gpolylla/mesh.h"

#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

using std::string;
using std::vector;

// VERTEX

bool gpolylla::Vertex::operator==(const gpolylla::Vertex &other) const {
  return x == other.x && y == other.y && z == other.z;
}

std::ostream &gpolylla::operator<<(std::ostream &os,
                                   const gpolylla::Vertex &v) {
  os << v.idx_ << " at (" << v.x << ", " << v.y << ", " << v.z << ")";
  return os;
}

// EDGE

bool gpolylla::Edge::operator==(const gpolylla::Edge &other) const {
  return initial == other.initial && final == other.final;
}
std::ostream &gpolylla::operator<<(std::ostream &os, const gpolylla::Edge &e) {
  os << *e.initial << " -> " << *e.final;
  return os;
}
// FACE
bool gpolylla::Face::operator==(const gpolylla::Face &other) const {
  return vertices == other.vertices;
}
std::ostream &gpolylla::operator<<(std::ostream &os, const gpolylla::Face &f) {
  os << f.idx_ << " at " << *f.vertices[0] << " -- " << *f.vertices[1] << " -- "
     << *f.vertices[2];
  return os;
}
// TETRAHEDRON

bool gpolylla::Tetrahedron::operator==(
    const gpolylla::Tetrahedron &other) const {
  return vertices == other.vertices;
}

// MESH
namespace gpolylla {
void read_nodes(const string &filename, vector<Vertex> *vertices) {
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
    Vertex node(vertices->size());
    iss >> node.x >> node.y >> node.z;
    vertices->push_back(node);
  }
  file.close();
}

void read_tetrahedrons(const string &filename, vector<Tetrahedron> *tetras,
                       vector<Vertex> *vertices) {
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
  tetras->reserve(num_tetras);
  while (std::getline(file, line)) {
    std::istringstream iss(line);
    std::string token;
    iss >> token;
    if (token == "#") continue;
    Tetrahedron t;
    // t.idx = std::stoi(token);
    int v1, v2, v3, v4;
    iss >> v1 >> v2 >> v3 >> v4;
    t.vertices.push_back(&vertices->at(v1));
    t.vertices.push_back(&vertices->at(v2));
    t.vertices.push_back(&vertices->at(v3));
    t.vertices.push_back(&vertices->at(v4));
    tetras->push_back(t);
  }
  file.close();
}

void create_faces(vector<Face> *faces, vector<Tetrahedron> *tetras) {
  std::unordered_map<Face, std::vector<int>> map;
  for (int i = 0; i < tetras->size(); i++) {
    Tetrahedron &tetra = tetras->at(i);
    for (int j = 0; j < 4; j++) {
      Face f(map.size());
      f.vertices.push_back(tetra.vertices[j % 4]);
      f.vertices.push_back(tetra.vertices[(j + 1) % 4]);
      f.vertices.push_back(tetra.vertices[(j + 2) % 4]);
      if (map.find(f) == map.end()) {
        map[f] = std::vector<int>();
      }
      map[f].push_back(i);
    }
  }

  for (auto &pair : map) {
    Face f = pair.first;
    if (pair.second.size() != 2) {
      std::cerr << f << " without 2 tetrahedrons (" << pair.second.size() << ")"
                << std::endl;
      // std::cerr << "Total faces: " << map.size() << std::endl;
      // for (auto &pair : map) {
      //   std::cerr << pair.first << ": ";
      //   for (auto &idx: pair.second) {
      //     std::cerr << idx << " ";
      //   }
      //   std::cerr << std::endl;
      // }
      exit(2);
      // 32 29 58
    }
    int init = pair.second[0];
    int final = pair.second[1];
    f.init = &tetras->at(init);
    f.final = &tetras->at(final);
    tetras->at(init).faces.push_back(&f);
    tetras->at(final).faces.push_back(&f);
    faces->push_back(f);
  }
}

TetrahedronMesh::TetrahedronMesh(const string &node_filename,
                                 // const string &face_filename,
                                 const string &ele_filename) {
  std::cout << "Constructing mesh..." << std::endl;
  // EdgeMatrix e_matrix;
  // FaceMatrix f_matrix;
  // emf edges_matrix_faces;
  std::cout << "Getting vertex from node file..." << std::endl;
  auto t0 = std::chrono::high_resolution_clock::now();
  read_nodes(node_filename, &vertices_);
  std::cout << "Getting tetrahedrons from ele file..." << std::endl;
  auto t1 = std::chrono::high_resolution_clock::now();
  read_tetrahedrons(ele_filename, &tetrahedrons_, &vertices_);
  std::cout << "Procesing faces..." << std::endl;
  auto t2 = std::chrono::high_resolution_clock::now();
  create_faces(&faces_, &tetrahedrons_);
  std::cout << "Procesing edges..." << std::endl;
  // save_faces(face_filename, &e_matrix, nodes.size(), &faces, &f_matrix);
  // std::cout << "Procesing edges..." << std::endl;
  // auto t2 = std::chrono::high_resolution_clock::now();
  // save_edges(&e_matrix, &faces, &edges);
  // std::cout << "Reading tetra file..." << std::endl;
  // auto t3 = std::chrono::high_resolution_clock::now();
  // save_tetra(ele_filename, &tetras);
  // std::cout << "Processing faces with tetrahedrons..." << std::endl;
  // auto t4 = std::chrono::high_resolution_clock::now();
  // asign_faces_and_edges_to_tetras(&tetras, &faces, &edges);
  // auto t5 = std::chrono::high_resolution_clock::now();
  // for (auto &tetra : tetras) {
  //   asign_neighs(&tetra, &faces);
  // }
  // auto t6 = std::chrono::high_resolution_clock::now();
  // // for (auto &face : faces) {
  // //   if (face.is_boundary) {
  // //     for (int ei : face.edges) {
  // //       if (face.ni != -1) {
  // //         edges[ei].first_tetra = face.ni;
  // //       } else {
  // //         edges[ei].first_tetra = face.nf;
  // //       }
  // //     }
  // //   }
  // // }
  //
  // std::cout
  //     << "Time for nodes: "
  //     << std::chrono::duration_cast<std::chrono::milliseconds>(t1 -
  //     t0).count()
  //     << " ms" << std::endl;
  // std::cout
  //     << "Time for faces: "
  //     << std::chrono::duration_cast<std::chrono::milliseconds>(t2 -
  //     t1).count()
  //     << " ms" << std::endl;
  // std::cout
  //     << "Time for edges: "
  //     << std::chrono::duration_cast<std::chrono::milliseconds>(t3 -
  //     t2).count()
  //     << " ms" << std::endl;
  // std::cout
  //     << "Time for tetras: "
  //     << std::chrono::duration_cast<std::chrono::milliseconds>(t4 -
  //     t3).count()
  //     << " ms" << std::endl;
  // std::cout
  //     << "Time for processing faces: "
  //     << std::chrono::duration_cast<std::chrono::milliseconds>(t5 -
  //     t4).count()
  //     << " ms" << std::endl;
  // std::cout
  //     << "Time for asigning neighbours: "
  //     << std::chrono::duration_cast<std::chrono::milliseconds>(t6 -
  //     t5).count()
  //     << " ms" << std::endl;
  // std::cout
  //     << "Total time: "
  //     << std::chrono::duration_cast<std::chrono::milliseconds>(t6 -
  //     t0).count()
  //     << " ms" << std::endl;
}

}  // namespace gpolylla
