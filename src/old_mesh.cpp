#include "GPolylla/mesh.h"

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <ostream>
#include <set>
#include <sstream>
#include <unordered_map>
#include <vector>

// template <typename T>
// bool check_in(const std::vector<T> &vec, const T &elem) {
//   return std::find(vec.begin(), vec.end(), elem) != vec.end();
// }

namespace GPolylla {
using std::pair;
using std::string;
using std::unordered_map;
using std::vector;

// Polyhedron
// Polyhedron::Polyhedron() : was_repaired(false) {}

// Vertex
// Vertex::Vertex() : id(-1), x(0.0f), y(0.0f), z(0.0f) {}

std::ostream &operator<<(std::ostream &os, const Vertex &v) {
  os << "(Vertex " << v.idx << " x: " << v.x << " y: " << v.y << " z: " << v.z
     << ")";
  return os;
}

// Face
void get_edges_info(vector<pair<int, int>> *info, const Face &f) {
  info->push_back(std::make_pair(f.v1, f.v2));
  info->push_back(std::make_pair(f.v2, f.v3));
  info->push_back(std::make_pair(f.v3, f.v1));
}
// Face::Face()
//     : id(-1), v1(-1), v2(-1), v3(-1), n1(-1), n2(-1), is_boundary(false) {}
std::ostream &operator<<(std::ostream &os, const Face &f) {
  os << "(Face " << f.idx << " Vertex 1: " << f.v1 << " Vertex 2: " << f.v2
     << " Vertex 3: " << f.v3 << " Edges: {";
  for (auto &e : f.edges) os << e << ", ";
  os << "} Neighs: {" << f.ni << ", " << f.nf << "})";
  return os;
}

// Edge
// Edge::Edge() : id(-1), vi(-1), vf(-1), first_tetra(-1), length(0.0f) {}
std::ostream &operator<<(std::ostream &os, const Edge &e) {
  os << "(Edge " << e.idx << " Vertex i: " << e.vi << " Vertex f: " << e.vf
     << " Faces: {";
  for (auto &f : e.faces) os << f << ", ";
  os << "})";
  return os;
}

// bool Edge::compare(int v1, int v2) const {
//   return (this->vi == v1 && this->vf == v2) ||
//          (this->vi == v2 && this->vf == v1);
// }

// Tetrahedron
// Tetrahedron::Tetrahedron()
//     : id(-1), v1(-1), v2(-1), v3(-1), v4(-1), is_boundary(false) {}
std::ostream &operator<<(std::ostream &os, const Tetrahedron &t) {
  os << "(Tetra " << t.idx << " Vertex 1: " << t.v1 << " Vertex 2: " << t.v2
     << " Vertex 3: " << t.v3 << " Vertex 4: " << t.v4 << " Faces: {";
  for (auto &f : t.faces) os << f << ", ";
  os << "} Neighs: {";
  for (auto &n : t.neighs) os << n << ", ";
  os << "})";
  return os;
}

// Tetrahedron Mesh
using EdgeMatrix = vector<pair<vector<int>, vector<vector<int>>>>;
using FaceMatrix = vector<vector<unordered_map<int, int>>>;
// the matrix saves the info of the conectivity of the edges
// first is a list of vertices, second is a list of list of faces

void save_vertex(const std::string &filename, vector<Vertex> *vertices,
                 EdgeMatrix *mat) {
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
  mat->resize(num_nodes);
  while (std::getline(file, line)) {
    std::istringstream iss(line);
    std::string token;
    iss >> token;
    if (token == "#") continue;
    Vertex node;
    node.idx = std::stoi(token);
    iss >> node.x >> node.y >> node.z;
    vertices->push_back(node);
    mat->push_back(std::make_pair(vector<int>(), vector<vector<int>>()));
  }
  file.close();
}

void save_faces(const std::string &filename, EdgeMatrix *e_matrix,
                std::size_t n, vector<Face> *faces, FaceMatrix *f_matrix) {
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
  faces->reserve(num_faces);
  *f_matrix = FaceMatrix(n, vector<unordered_map<int, int>>(n));
  while (std::getline(file, line)) {
    std::istringstream iss(line);
    std::string token;
    iss >> token;
    // if is a comment
    if (token == "#") continue;
    // Create face
    Face face;
    face.idx = std::stoi(token);
    iss >> face.v1 >> face.v2 >> face.v3;
    faces->push_back(face);

    // Save edges data
    bool saved_e1 = false;
    bool saved_e2 = false;
    bool saved_e3 = false;

    // TODO: Optimize this, maybe using a map
    if (std::find(e_matrix->at(face.v1).first.begin(),
                  e_matrix->at(face.v1).first.end(),
                  face.v2) == e_matrix->at(face.v1).first.end() &&
        std::find(e_matrix->at(face.v2).first.begin(),
                  e_matrix->at(face.v2).first.end(),
                  face.v1) == e_matrix->at(face.v2).first.end()) {
      e_matrix->at(face.v1).first.push_back(face.v2);
      vector<int> temp;
      temp.push_back(face.idx);
      e_matrix->at(face.v1).second.push_back(temp);
      saved_e1 = true;
    }

    if (std::find(e_matrix->at(face.v2).first.begin(),
                  e_matrix->at(face.v2).first.end(),
                  face.v3) == e_matrix->at(face.v2).first.end() &&
        std::find(e_matrix->at(face.v3).first.begin(),
                  e_matrix->at(face.v3).first.end(),
                  face.v2) == e_matrix->at(face.v3).first.end()) {
      e_matrix->at(face.v2).first.push_back(face.v3);
      vector<int> temp;
      temp.push_back(face.idx);
      e_matrix->at(face.v2).second.push_back(temp);
      saved_e2 = true;
    }

    if (std::find(e_matrix->at(face.v3).first.begin(),
                  e_matrix->at(face.v3).first.end(),
                  face.v1) == e_matrix->at(face.v3).first.end() &&
        std::find(e_matrix->at(face.v1).first.begin(),
                  e_matrix->at(face.v1).first.end(),
                  face.v3) == e_matrix->at(face.v1).first.end()) {
      e_matrix->at(face.v3).first.push_back(face.v1);
      vector<int> temp;
      temp.push_back(face.idx);
      e_matrix->at(face.v3).second.push_back(temp);
      saved_e3 = true;
    }

    if (!saved_e1) {
      if (auto it = std::find(e_matrix->at(face.v1).first.begin(),
                              e_matrix->at(face.v1).first.end(), face.v2);
          it != e_matrix->at(face.v1).first.end()) {
        int index = std::distance(e_matrix->at(face.v1).first.begin(), it);
        e_matrix->at(face.v1).second[index].push_back(face.idx);
      } else {
        int index = std::distance(
            e_matrix->at(face.v2).first.begin(),
            std::find(e_matrix->at(face.v2).first.begin(),
                      e_matrix->at(face.v2).first.end(), face.v1));
        e_matrix->at(face.v2).second[index].push_back(face.idx);
      }
    }
    if (!saved_e2) {
      if (auto it = std::find(e_matrix->at(face.v2).first.begin(),
                              e_matrix->at(face.v2).first.end(), face.v3);
          it != e_matrix->at(face.v2).first.end()) {
        int index = std::distance(e_matrix->at(face.v2).first.begin(), it);
        e_matrix->at(face.v2).second[index].push_back(face.idx);
      } else {
        int index = std::distance(
            e_matrix->at(face.v3).first.begin(),
            std::find(e_matrix->at(face.v3).first.begin(),
                      e_matrix->at(face.v3).first.end(), face.v2));
        e_matrix->at(face.v3).second[index].push_back(face.idx);
      }
    }

    if (!saved_e3) {
      if (auto it = std::find(e_matrix->at(face.v3).first.begin(),
                              e_matrix->at(face.v3).first.end(), face.v1);
          it != e_matrix->at(face.v3).first.end()) {
        int index = std::distance(e_matrix->at(face.v3).first.begin(), it);
        e_matrix->at(face.v3).second[index].push_back(face.idx);
      } else {
        int index = std::distance(
            e_matrix->at(face.v1).first.begin(),
            std::find(e_matrix->at(face.v1).first.begin(),
                      e_matrix->at(face.v1).first.end(), face.v3));
        e_matrix->at(face.v1).second[index].push_back(face.idx);
      }
    }

    // vector<pair<int, int>> edges_info;
    // get_edges_info(&edges_info, face);
    // for (auto &edge_info : edges_info) {
    //   auto itvi = std::find(emv->at(edge_info.first).begin(),
    //                         emv->at(edge_info.first).end(),
    //                         edge_info.second);
    //   auto itvf = std::find(emv->at(edge_info.second).begin(),
    //                         emv->at(edge_info.second).end(),
    //                         edge_info.first);
    //   if (itvi == emv->at(edge_info.first).end() &&
    //       itvf == emv->at(edge_info.second).end()) {
    //     // New edge
    //     emv->at(edge_info.first).push_back(edge_info.second);
    //     vector<int> fs;
    //     fs.push_back(face.id);
    //     emf->at(edge_info.first).push_back(fs);
    //   } else {
    //     // Already exists
    //     if (itvi != emv->at(edge_info.first).end()) {
    //       // Is missing in the first vertex info
    //       int index = std::distance(emv->at(edge_info.first).begin(), itvi);
    //       emf->at(edge_info.first)[index].push_back(face.id);
    //     } else {
    //       // Is missing in the second vertex info
    //       int index = std::distance(emv->at(edge_info.second).begin(), itvf);
    //       emf->at(edge_info.second)[index].push_back(face.id);
    //     }
    //   }
    // }
  }
  file.close();
}

void save_edges(EdgeMatrix *mat, vector<Face> *faces, vector<Edge> *edges) {
  int ei = 0;
  for (int vi = 0; vi < mat->size(); vi++) {
    for (int j = 0; j < mat->at(vi).first.size(); j++) {
      int vf = mat->at(vi).first[j];
      auto efaces = mat->at(vi).second[j];
      Edge e;
      e.idx = ei++;
      e.vi = vi;
      e.vf = vf;
      e.faces = efaces;
      edges->push_back(e);
      for (int fi : e.faces) {
        auto &face = faces->at(fi);
        face.edges.push_back(e.idx);
      }
    }
  }
}

void save_tetra(const std::string &filename, vector<Tetrahedron> *tetras) {
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
    t.idx = std::stoi(token);
    iss >> t.v1 >> t.v2 >> t.v3 >> t.v4;
    tetras->push_back(t);
  }
  file.close();
}

void asign_faces_and_edges_to_tetras(vector<Tetrahedron> *tetras,
                                     vector<Face> *faces, vector<Edge> *edges) {
  for (auto &f : *faces) {
    for (auto &t : *tetras) {
      if (f.neighs.size() == 2) break;
      std::set<int> ts = {t.v1, t.v2, t.v3, t.v4};
      std::set<int> fs = {f.v1, f.v2, f.v3};
      std::set<int> inter;
      std::set_intersection(ts.begin(), ts.end(), fs.begin(), fs.end(),
                            std::inserter(inter, inter.begin()));
      if (inter == fs) {
        t.faces.push_back(f.idx);
        f.neighs.push_back(t.idx);
        for (int ei : f.edges) {
          if (std::find(t.edges.begin(), t.edges.end(), ei) == t.edges.end())
            t.edges.push_back(ei);
          // edges->at(ei).tetrahedrons.push_back(t.idx);
        }
      }
    }
  }
}

void asign_neighs(Tetrahedron *t, vector<Face> *faces) {
  std::vector<int> neighs;
  for (int fi : t->faces) {
    auto &face = faces->at(fi);
    if (face.neighs.size() < 2) {
      t->is_boundary = true;
      face.is_boundary = true;
    }

    face.ni = face.neighs[0];
    if (face.ni != t->idx) {
      neighs.push_back(face.ni);
    }
    if (!face.is_boundary) {
      face.nf = face.neighs[1];
      if (face.nf != t->idx) {
        neighs.push_back(face.nf);
      }
    } else {
      neighs.push_back(-1);
    }
    t->neighs = neighs;
  }
}

void TetrahedronMesh::construct_tetrahedral_mesh(
    const std::string &node_filename, const std::string &face_filename,
    const std::string &ele_filename) {
  std::cout << "Constructing mesh from files..." << std::endl;
  EdgeMatrix e_matrix;
  FaceMatrix f_matrix;
  // emf edges_matrix_faces;
  std::cout << "Reading vertex file..." << std::endl;
  auto t0 = std::chrono::high_resolution_clock::now();
  save_vertex(node_filename, &nodes, &e_matrix);
  std::cout << "Reading face file..." << std::endl;
  auto t1 = std::chrono::high_resolution_clock::now();
  save_faces(face_filename, &e_matrix, nodes.size(), &faces, &f_matrix);
  std::cout << "Procesing edges..." << std::endl;
  auto t2 = std::chrono::high_resolution_clock::now();
  save_edges(&e_matrix, &faces, &edges);
  std::cout << "Reading tetra file..." << std::endl;
  auto t3 = std::chrono::high_resolution_clock::now();
  save_tetra(ele_filename, &tetras);
  std::cout << "Processing faces with tetrahedrons..." << std::endl;
  auto t4 = std::chrono::high_resolution_clock::now();
  asign_faces_and_edges_to_tetras(&tetras, &faces, &edges);
  auto t5 = std::chrono::high_resolution_clock::now();
  for (auto &tetra : tetras) {
    asign_neighs(&tetra, &faces);
  }
  auto t6 = std::chrono::high_resolution_clock::now();
  // for (auto &face : faces) {
  //   if (face.is_boundary) {
  //     for (int ei : face.edges) {
  //       if (face.ni != -1) {
  //         edges[ei].first_tetra = face.ni;
  //       } else {
  //         edges[ei].first_tetra = face.nf;
  //       }
  //     }
  //   }
  // }

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
      << "Time for processing faces: "
      << std::chrono::duration_cast<std::chrono::milliseconds>(t5 - t4).count()
      << " ms" << std::endl;
  std::cout
      << "Time for asigning neighbours: "
      << std::chrono::duration_cast<std::chrono::milliseconds>(t6 - t5).count()
      << " ms" << std::endl;
  std::cout
      << "Total time: "
      << std::chrono::duration_cast<std::chrono::milliseconds>(t6 - t0).count()
      << " ms" << std::endl;
}

// void TetrahedronMesh::read_face_file(const std::string &filename, emv *emv,
//                                      emf *emf) {
//   std::cout << "Reading face file: " << filename << std::endl;
//   std::ifstream file(filename);
//   if (!file.is_open()) {
//     std::cerr << "Unable to open file: " << filename << std::endl;
//     exit(1);
//   }
//   std::string line;
//   std::getline(file, line);
//   std::istringstream iss(line);
//   int num_faces;
//   iss >> num_faces;
//   faces.reserve(num_faces);
//   while (std::getline(file, line)) {
//     std::istringstream iss(line);
//     std::string token;
//     iss >> token;
//     if (token == "#") continue;
//     Face face;
//     face.id = std::stoi(token);
//     iss >> face.v1 >> face.v2 >> face.v3;
//     // int bm;
//     // iss >> bm;
//     // face.is_boundary = (bm == 1 || bm == -1);
//     // iss >> face.n1 >> face.n2;
//     faces.push_back(face);
//
//     // Save edges data
//     vector<pair<int, int>> edges_info;
//     get_edges_info(&edges_info, face);
//     for (auto &edge_info : edges_info) {
//       auto itvi = std::find(emv->at(edge_info.first).begin(),
//                             emv->at(edge_info.first).end(),
//                             edge_info.second);
//       auto itvf = std::find(emv->at(edge_info.second).begin(),
//                             emv->at(edge_info.second).end(),
//                             edge_info.first);
//       if (itvi == emv->at(edge_info.first).end() &&
//           itvf == emv->at(edge_info.second).end()) {
//         // New edge
//         emv->at(edge_info.first).push_back(edge_info.second);
//         vector<int> fs;
//         fs.push_back(face.id);
//         emf->at(edge_info.first).push_back(fs);
//       } else {
//         // Already exists
//         if (itvi != emv->at(edge_info.first).end()) {
//           // Is missing in the first vertex info
//           int index = std::distance(emv->at(edge_info.first).begin(), itvi);
//           emf->at(edge_info.first)[index].push_back(face.id);
//         } else {
//           // Is missing in the second vertex info
//           int index = std::distance(emv->at(edge_info.second).begin(), itvf);
//           emf->at(edge_info.second)[index].push_back(face.id);
//         }
//       }
//     }
//   }
//   file.close();
// }

// void TetrahedronMesh::read_ele_file(const std::string &filename) {
//   std::cout << "Reading ele file: " << filename << std::endl;
//   std::ifstream file(filename);
//   if (!file.is_open()) {
//     std::cerr << "Unable to open file: " << filename << std::endl;
//     exit(1);
//   }
//   std::string line;
//   std::getline(file, line);
//   std::istringstream iss(line);
//   int num_tetras;
//   iss >> num_tetras;
//   tetras.reserve(num_tetras);
//   while (std::getline(file, line)) {
//     std::istringstream iss(line);
//     std::string token;
//     iss >> token;
//     if (token == "#") continue;
//     Tetrahedron t;
//     t.id = std::stoi(token);
//     iss >> t.v1 >> t.v2 >> t.v3 >> t.v4;
//     tetras.push_back(t);
//   }
//   file.close();
// }

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
// void TetrahedronMesh::read_edge_file(emv *emv, emf *emf) {
//   int ei = 0;
//   for (int vi = 0; vi < emv->size(); vi++) {
//     for (int j = 0; j < emv->at(vi).size(); j++) {
//       int vf = emv->at(vi)[j];
//       auto efaces = emf->at(vi)[j];
//       Edge e;
//       e.id = ei++;
//       e.vi = vi;
//       e.vf = vf;
//       e.faces = efaces;
//       edges.push_back(e);
//       for (int fi : e.faces) {
//         auto &face = faces[fi];
//         face.edges.push_back(e.id);
//       }
//     }
//   }
// }

// void TetrahedronMesh::asign_faces_to_tetras() {
//   for (auto &f : faces) {
//     for (auto &t : tetras) {
//       std::set<int> ts = {t.v1, t.v2, t.v3, t.v4};
//       std::set<int> fs = {f.v1, f.v2, f.v3};
//       std::set<int> inter;
//       std::set_intersection(ts.begin(), ts.end(), fs.begin(), fs.end(),
//                             std::inserter(inter, inter.begin()));
//       if (inter == fs) {
//         t.faces.push_back(f.id);
//         f.neighs.push_back(t.id);
//       }
//     }
//   }
// }

// void TetrahedronMesh::asign_neighs(Tetrahedron *t) {
//   std::vector<int> neighs;
//   for (int fi : t->faces) {
//     auto &face = faces[fi];
//     if (face.neighs.size() < 2) {
//       t->is_boundary = true;
//       face.is_boundary = true;
//     }
//
//     face.ni = face.neighs[0];
//     if (face.ni != t->id) {
//       neighs.push_back(face.ni);
//     }
//     if (!face.is_boundary) {
//       face.nf = face.neighs[1];
//       if (face.nf != t->id) {
//         neighs.push_back(face.nf);
//       }
//     } else {
//       neighs.push_back(-1);
//     }
//     t->neighs = neighs;
//   }
// }

TetrahedronMesh::TetrahedronMesh(const std::string &node_filename,
                                 const std::string &face_filename,
                                 const std::string &ele_filename) {
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
