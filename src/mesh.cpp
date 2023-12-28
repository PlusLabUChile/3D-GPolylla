#include "GPolylla/mesh.h"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <ostream>
#include <set>
#include <sstream>

// template <typename T>
// bool check_in(const std::vector<T> &vec, const T &elem) {
//   return std::find(vec.begin(), vec.end(), elem) != vec.end();
// }

namespace GPolylla {
using std::pair;
using std::string;
using std::vector;

// Polyhedron
Polyhedron::Polyhedron() : was_repaired(false) {}

// Vertex
Vertex::Vertex() : id(-1), x(0.0f), y(0.0f), z(0.0f) {}

std::ostream &operator<<(std::ostream &os, const Vertex &v) {
  os << "(Vertex " << v.id << " x: " << v.x << " y: " << v.y << " z: " << v.z
     << ")";
  return os;
}

// Face
void get_edges_info(vector<pair<int, int>> *info, const Face &f) {
  info->push_back(std::make_pair(f.v1, f.v2));
  info->push_back(std::make_pair(f.v2, f.v3));
  info->push_back(std::make_pair(f.v3, f.v1));
}
Face::Face()
    : id(-1), v1(-1), v2(-1), v3(-1), n1(-1), n2(-1), is_boundary(false) {}
std::ostream &operator<<(std::ostream &os, const Face &f) {
  os << "(Face " << f.id << " Vertex 1: " << f.v1 << " Vertex 2: " << f.v2
     << " Vertex 3: " << f.v3 << " Edges: {";
  for (auto &e : f.edges) os << e << ", ";
  os << "} Neighs: {" << f.n1 << ", " << f.n2 << "})";
  return os;
}

// Edge
Edge::Edge() : id(-1), vi(-1), vf(-1), first_tetra(-1), length(0.0f) {}
std::ostream &operator<<(std::ostream &os, const Edge &e) {
  os << "(Edge " << e.id << " Vertex i: " << e.vi << " Vertex f: " << e.vf
     << " Faces: {";
  for (auto &f : e.faces) os << f << ", ";
  os << "})";
  return os;
}

bool Edge::compare(int v1, int v2) const {
  return (this->vi == v1 && this->vf == v2) ||
         (this->vi == v2 && this->vf == v1);
}

// Tetrahedron
Tetrahedron::Tetrahedron()
    : id(-1), v1(-1), v2(-1), v3(-1), v4(-1), is_boundary(false) {}
std::ostream &operator<<(std::ostream &os, const Tetrahedron &t) {
  os << "(Tetra " << t.id << " Vertex 1: " << t.v1 << " Vertex 2: " << t.v2
     << " Vertex 3: " << t.v3 << " Vertex 4: " << t.v4 << " Faces: {";
  for (auto &f : t.faces) os << f << ", ";
  os << "} Neighs: {";
  for (auto &n : t.neighs) os << n << ", ";
  os << "})";
  return os;
}

// Tetrahedron Mesh
using emv = vector<vector<int>>;
using emf = vector<vector<vector<int>>>;
void TetrahedronMesh::construct_tetrahedral_mesh(
    const std::string &node_filename, const std::string &face_filename,
    const std::string &ele_filename) {
  emv edges_matrix_vertex;
  emf edges_matrix_faces;
  auto t0 = std::chrono::high_resolution_clock::now();
  read_node_file(node_filename, &edges_matrix_vertex, &edges_matrix_faces);
  auto t1 = std::chrono::high_resolution_clock::now();
  read_face_file(face_filename, &edges_matrix_vertex, &edges_matrix_faces);
  auto t2 = std::chrono::high_resolution_clock::now();
  read_edge_file(&edges_matrix_vertex, &edges_matrix_faces);
  auto t3 = std::chrono::high_resolution_clock::now();
  read_ele_file(ele_filename);
  auto t4 = std::chrono::high_resolution_clock::now();

  asign_faces_to_tetras();
  auto t5 = std::chrono::high_resolution_clock::now();
  for (auto &face : faces) {
    if (face.is_boundary) {
      for (int ei : face.edges) {
        if (face.n1 != -1) {
          edges[ei].first_tetra = face.n1;
        } else {
          edges[ei].first_tetra = face.n2;
        }
      }
    }
  }

  auto t6 = std::chrono::high_resolution_clock::now();
  for (auto &tetra : tetras) {
    asign_neighs(&tetra);
  }
  auto t7 = std::chrono::high_resolution_clock::now();
  std::cout
      << "Time for nodes: "
      << std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count()
      << " ms" << std::endl;
  std::cout
      << "Time for faces: "
      << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count()
      << " ms" << std::endl;
  std::cout
      << "Time for edges: "
      << std::chrono::duration_cast<std::chrono::milliseconds>(t3 - t2).count()
      << " ms" << std::endl;
  std::cout
      << "Time for tetras: "
      << std::chrono::duration_cast<std::chrono::milliseconds>(t4 - t3).count()
      << " ms" << std::endl;
  std::cout
      << "Time for face to tetras: "
      << std::chrono::duration_cast<std::chrono::milliseconds>(t5 - t4).count()
      << " ms" << std::endl;
  std::cout
      << "Time for edges to faces: "
      << std::chrono::duration_cast<std::chrono::milliseconds>(t6 - t5).count()
      << " ms" << std::endl;
  std::cout
      << "Time for tetras to neighs: "
      << std::chrono::duration_cast<std::chrono::milliseconds>(t7 - t6).count()
      << " ms" << std::endl;
  std::cout
      << "Total time: "
      << std::chrono::duration_cast<std::chrono::milliseconds>(t7 - t0).count()
      << " ms" << std::endl;
}

void TetrahedronMesh::read_node_file(const std::string &filename, emv *emv,
                                     emf *emf) {
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
  emv->resize(num_nodes);
  emf->resize(num_nodes);
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

void TetrahedronMesh::read_face_file(const std::string &filename, emv *emv,
                                     emf *emf) {
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
  while (std::getline(file, line)) {
    std::istringstream iss(line);
    std::string token;
    iss >> token;
    if (token == "#") continue;
    Face face;
    face.id = std::stoi(token);
    iss >> face.v1 >> face.v2 >> face.v3;
    // int bm;
    // iss >> bm;
    // face.is_boundary = (bm == 1 || bm == -1);
    // iss >> face.n1 >> face.n2;
    faces.push_back(face);

    // Save edges data
    vector<pair<int, int>> edges_info;
    get_edges_info(&edges_info, face);
    for (auto &edge_info : edges_info) {
      auto itvi = std::find(emv->at(edge_info.first).begin(),
                            emv->at(edge_info.first).end(), edge_info.second);
      auto itvf = std::find(emv->at(edge_info.second).begin(),
                            emv->at(edge_info.second).end(), edge_info.first);
      if (itvi == emv->at(edge_info.first).end() &&
          itvf == emv->at(edge_info.second).end()) {
        // New edge
        emv->at(edge_info.first).push_back(edge_info.second);
        vector<int> fs;
        fs.push_back(face.id);
        emf->at(edge_info.first).push_back(fs);
      } else {
        // Already exists
        if (itvi != emv->at(edge_info.first).end()) {
          // Is missing in the first vertex info
          int index = std::distance(emv->at(edge_info.first).begin(), itvi);
          emf->at(edge_info.first)[index].push_back(face.id);
        } else {
          // Is missing in the second vertex info
          int index = std::distance(emv->at(edge_info.second).begin(), itvf);
          emf->at(edge_info.second)[index].push_back(face.id);
        }
      }
    }
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

// void TetrahedronMesh::read_edge_file(const std::string &filename, emv *emv,
//                                      emf *emf) {
//   std::cout << "Reading edge file: " << filename << std::endl;
//   std::ifstream file(filename);
//   if (!file.is_open()) {
//     std::cerr << "Unable to open file: " << filename << std::endl;
//     exit(1);
//   }
//   std::string line;
//   std::getline(file, line);
//   std::istringstream iss(line);
//   int num_edges;
//   iss >> num_edges;
//   tetras.reserve(num_edges);
//   while (std::getline(file, line)) {
//     std::istringstream iss(line);
//     std::string token;
//     iss >> token;
//     if (token == "#") continue;
//     Edge e;
//     e.id = std::stoi(token);
//     iss >> e.v1 >> e.v2;
//     int bm;
//     iss >> bm;
//     e.is_boundary = bm == 1;
//     iss >> e.first_tetra;
//     edges.push_back(e);
//   }
//   file.close();
// }
//
void TetrahedronMesh::read_edge_file(emv *emv, emf *emf) {
  int ei = 0;
  for (int vi = 0; vi < emv->size(); vi++) {
    for (int j = 0; j < emv->at(vi).size(); j++) {
      int vf = emv->at(vi)[j];
      auto efaces = emf->at(vi)[j];
      Edge e;
      e.id = ei++;
      e.vi = vi;
      e.vf = vf;
      e.faces = efaces;
      edges.push_back(e);
      for (int fi : e.faces) {
        auto &face = faces[fi];
        face.edges.push_back(e.id);
      }
    }
  }
}

void TetrahedronMesh::asign_faces_to_tetras() {
  for (auto &f : faces) {
    for (auto &t : tetras) {
      std::set<int> ts = {t.v1, t.v2, t.v3, t.v4};
      std::set<int> fs = {f.v1, f.v2, f.v3};
      std::set<int> inter;
      std::set_intersection(ts.begin(), ts.end(), fs.begin(), fs.end(),
                            std::inserter(inter, inter.begin()));
      if (inter == fs) {
        t.faces.push_back(f.id);
        f.neighs.push_back(t.id);
      }
    }
  }
}

void TetrahedronMesh::asign_neighs(Tetrahedron *t) {
  std::vector<int> neighs;
  for (int fi : t->faces) {
    auto &face = faces[fi];
    if (face.neighs.size() < 2) {
      t->is_boundary = true;
      face.is_boundary = true;
    }

    face.n1 = face.neighs[0];
    if (face.n1 != t->id) {
      neighs.push_back(face.n1);
    }
    if (!face.is_boundary) {
      face.n2 = face.neighs[1];
      if (face.n2 != t->id) {
        neighs.push_back(face.n2);
      }
    } else {
      neighs.push_back(-1);
    }
    t->neighs = neighs;
  }
}

TetrahedronMesh::TetrahedronMesh(const std::string &node_filename,
                                 const std::string &face_filename,
                                 const std::string &ele_filename) {
  std::cout << "Constructing mesh from files..." << std::endl;
  construct_tetrahedral_mesh(node_filename, face_filename, ele_filename);
}

Face &TetrahedronMesh::get_face(int id) { return faces[id]; }
Tetrahedron &TetrahedronMesh::get_tetra(int id) {
  // std::cout << "Trying to get " << id << std::endl;
  return tetras[id];
}
Edge &TetrahedronMesh::get_edge(int id) { return edges[id]; }
Vertex &TetrahedronMesh::get_vertex(int id) { return nodes[id]; }

vector<vector<double>> TetrahedronMesh::get_vertices() const {
  vector<vector<double>> vertices;
  for (auto &v : nodes) {
    vertices.push_back({v.x, v.y, v.z});
  }
  return vertices;
}

vector<vector<int>> TetrahedronMesh::get_faces() const {
  vector<vector<int>> faces;
  for (auto &f : this->faces) {
    faces.push_back({f.v1, f.v2, f.v3});
  }
  return faces;
}
vector<vector<int>> TetrahedronMesh::get_tetrahedrons() const {
  vector<vector<int>> tetrahedrons;
  for (auto &t : tetras) {
    tetrahedrons.push_back({t.v1, t.v2, t.v3, t.v4});
  }
  return tetrahedrons;
}

int TetrahedronMesh::num_faces() const { return faces.size(); }
int TetrahedronMesh::num_tetrahedrons() const { return tetras.size(); }
int TetrahedronMesh::num_edges() const { return edges.size(); }
int TetrahedronMesh::num_nodes() const { return nodes.size(); }

void TetrahedronMesh::display_info() const {
  std::cout << "Number of nodes: " << num_nodes() << std::endl;
  std::cout << "Number of faces: " << num_faces() << std::endl;
  std::cout << "Number of tetrahedrons: " << num_tetrahedrons() << std::endl;
  std::cout << "Number of edges: " << num_edges() << std::endl;
}

void TetrahedronMesh::write_off(const string &filename) const {
  std::cout << "Writing mesh to: " << filename << std::endl;
  std::ofstream out(filename);
  out << "OFF" << std::endl;
  out << num_nodes() << " " << num_faces() << " 0" << std::endl;
  for (auto &v : nodes) {
    out << v.x << " " << v.y << " " << v.z << std::endl;
  }
  for (auto &f : faces) {
    out << "3 " << f.v1 << " " << f.v2 << " " << f.v3 << std::endl;
  }
  out.close();
}
}  // namespace GPolylla
