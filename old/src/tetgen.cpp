#include <gpolylla/io.h>

#include <algorithm>
#include <fstream>
#include <sstream>
#include <unordered_set>

#include "gpolylla/mesh.h"
#include "gpolylla/utils.h"

namespace gpolylla {
using Eigen::Vector3d;
using std::cerr;
using std::cout;
using std::endl;
using std::ostream;
using std::string;

TetraMesh TetgenReader::read(const string &name) {
  int ok = 1;
  mesh = TetraMesh();
  cout << "Constructing mesh from files...\n";
  cout << "Reading vertex file (.node)...\n";
  ok &= loadNode(name + ".node");
  cout << "Reading tetrahedron file (.ele)...\n";
  ok &= loadEle(name + ".ele");

  if (!ok) {
    cerr << ".ele and .node files are mandatory\n";
    return mesh;
  }

  cout << "Done!\n";
  return mesh;
}

int TetgenReader::loadNode(const string &filename) {
  std::ifstream nodeFile(filename);
  if (!nodeFile.is_open()) {
    cerr << "Unable to open file: " << filename << endl;
    return 0;
  }
  string line;
  std::getline(nodeFile, line);
  std::stringstream in(line);
  int numNodes = 0;
  in >> numNodes;
  mesh.vertices.reserve(numNodes);
  while (std::getline(nodeFile, line)) {
    in.str(line);
    string token;
    in >> token;
    if (token == "#") continue;
    // Vertex& node = _mesh.nodes[idx];
    //    int idx = std::stoi(token);
    // Vertex v;
    float x, y, z;
    in >> x >> y >> z;
    mesh.vertices.emplace_back(x, y, z);
    in.clear();
  }

  return 1;
}

int TetgenReader::loadEle(const string &filename) {
  std::ifstream eleFile(filename);
  if (!eleFile.is_open()) {
    cerr << "Unable to open file: " << filename << endl;
    return 0;
  }
  string line;
  std::getline(eleFile, line);
  std::stringstream in(line);
  int numTetras = 0;
  in >> numTetras;
  mesh.tetras.reserve(numTetras);
  while (std::getline(eleFile, line)) {
    in.str(line);
    string token;
    in >> token;
    if (token == "#") continue;
    //    int idx = std::stoi(token);
    int verts[4];
    in >> verts[0] >> verts[1] >> verts[2] >> verts[3];

    // sort
    std::sort(verts, verts + 4);
    mesh.tetras.emplace_back(verts[0], verts[1], verts[2], verts[3]);
    in.clear();
  }

  return 1;
}

// int TetgenReader::loadEdge(const string &filename) {
//   std::ifstream edgeFile(filename);
//   if (!edgeFile.is_open()) {
//     cerr << "Unable to open file: " << filename << endl;
//     return 0;
//   }
//   string line;
//   std::getline(edgeFile, line);
//   std::stringstream in(line);
//   int numEdges = 0;
//   in >> numEdges;
//   mesh.tetras.reserve(numEdges);
//   while (std::getline(edgeFile, line)) {
//     in.str(line);
//     string token;
//     in >> token;
//     if (token == "#") continue;
//     //    int idx = std::stoi(token);
//     int v0, v1;
//     in >> v0 >> v1;
//     // Edge e;
//     // e.vertices[0] = v0;
//     // e.vertices[1] = v1;
//     mesh.edges.emplace_back(v0, v1);
//     in.clear();
//   }
//
//   return 1;
// }
//
// int TetgenReader::loadFace(const string &filename) {
//   std::ifstream faceFile(filename);
//   if (!faceFile.is_open()) {
//     cerr << "Unable to open file: " << filename << endl;
//     return 0;
//   }
//   string line;
//   std::getline(faceFile, line);
//   std::stringstream in(line);
//   int numFaces = 0;
//   in >> numFaces;
//   mesh.faces.reserve(numFaces);
//   while (std::getline(faceFile, line)) {
//     in.str(line);
//     string token;
//     in >> token;
//     if (token == "#") continue;
//     // Face& face = _mesh->faces[idx];
//     //    int idx = std::stoi(token);
//     int v0, v1, v2;
//     in >> v0 >> v1 >> v2;
//     // Face f;
//     // f.vertices[0] = v0;
//     // f.vertices[1] = v1;
//     // f.vertices[2] = v2;
//     mesh.faces.emplace_back(v0, v1, v2);
//     in.clear();
//   }
//   return 1;
// }
//
// struct EdgeCompare {
//   bool operator()(const Edge &l, const Edge &r) const {
//     return l.vertices[0] < r.vertices[0] && l.vertices[1] < r.vertices[1];
//   }
// };
//
// struct FaceCompare {
//   bool operator()(const Face &l, const Face &r) const {
//     if (l.vertices[0] == r.vertices[0]) {
//       if (l.vertices[1] == r.vertices[1]) {
//         return l.vertices[2] < r.vertices[2];
//       } else
//         return l.vertices[1] < r.vertices[1];
//
//     } else
//       return l.vertices[0] < r.vertices[0];
//
//     // return *l.vertices[0] < *r.vertices[0] && *l.vertices[1] <
//     *r.vertices[1]
//     // &&
//     //        *l.vertices[2] < *r.vertices[2];
//   }
// };
//
// int TetgenReader::buildEdges() {
//   // create edges
//   std::unordered_set<Edge, Edge::Hash> uniqueEdges;
//
//   for (const auto &t : _mesh.tetras) {
//     auto verts = t.vertices;
//     std::sort(verts.begin(), verts.end());
//     for (int i = 0; i < 4; i++) {
//       for (int j = 0; j < i; j++) {
//         Edge e;
//         e.vertices[0] = verts[j];
//         e.vertices[1] = verts[i % 4];
//         // std::sort(e.vertices.begin(), e.vertices.end());
//         uniqueEdges.insert(e);
//       }
//     }
//     // Edge e0(0), e1(0), e2(0), e3(0);
//     // e0.vertices[0] = verts[0];
//     // e0.vertices[1] = verts[1];
//     // edgeSet.insert(e0);
//     // e1.vertices[0] = verts[1];
//     // e1.vertices[1] = verts[2];
//     // edgeSet.insert(e1);
//     // e2.vertices[0] = verts[2];
//     // e2.vertices[1] = verts[3];
//     // edgeSet.insert(e2);
//     // e3.vertices[0] = verts[3];
//     // e3.vertices[1] = verts[0];
//     // edgeSet.insert(e3);
//   }
//
//   int idx = 0;
//   for (const auto &old : uniqueEdges) {
//     Edge e;
//     e.vertices[0] = old.vertices[0];
//     e.vertices[1] = old.vertices[1];
//     _mesh.edges.push_back(e);
//     idx++;
//   }
//
//   return 1;
// }
//
// int TetgenReader::buildFaces() {
//   // create unique edges
//   // std::unordered_set<Face, Face::Hash> uniqueFaces;
//
//   for (int ti = 0; ti < _mesh.tetras.size(); ti++) {
//     auto &t = _mesh.tetras[ti];
//     auto verts = t.vertices;
//     std::sort(verts.begin(), verts.end());
//     for (int i = 0; i < 4; i++) {
//       Face f;
//       f.vertices[0] = verts[(i + 1) % 4];
//       f.vertices[1] = verts[(i + 2) % 4];
//       f.vertices[2] = verts[(i + 3) % 4];
//       auto n = normal(_mesh.vertices[f[0]], _mesh.vertices[f[1]],
//                       _mesh.vertices[f[2]]);
//
//       Vector3d intern =
//           (Vector3d)_mesh.vertices[verts[i]] -
//           (Vector3d)_mesh.vertices[f[0]];
//
//       if (intern.dot(n) > 0) {
//         f.vertices[0] = verts[(i + 3) % 4];
//         f.vertices[1] = verts[(i + 2) % 4];
//         f.vertices[2] = verts[(i + 1) % 4];
//       }
//
//       f.tetra = ti;
//       t.faces[i] = _mesh.faces.size();
//       _mesh.faces.push_back(f);
//     }
//   }
//   return 1;
// }
//
// //
// // TetgenBuilder::FaceKey::FaceKey(const Face& f) {
// //   vector<Vertex*> tmp;
// //   tmp = f.vertices;
// //   std::sort(tmp.begin(), tmp.end());
// //   v0 = tmp[0];
// //   v1 = tmp[1];
// //   v2 = tmp[2];
// // }
// //
// // TetgenBuilder::FaceKey::FaceKey(Vertex* v0, Vertex* v1, Vertex* v2)
// //     : v0(v0), v1(v1), v2(v2) {}
// //
// // bool TetgenBuilder::FaceKey::operator==(const FaceKey& o) const {
// //   return v0 == o.v0 && v1 == o.v1 && v2 == o.v2;
// // }
// //
// // std::size_t TetgenBuilder::FaceKey::Hash::operator()(const FaceKey& f)
// const
// // {
// //   std::size_t h0 = Vertex::Hash()(*f.v0);
// //   std::size_t h1 = Vertex::Hash()(*f.v1);
// //   std::size_t h2 = Vertex::Hash()(*f.v2);
// //   return h0 ^ (h1 << 1) ^ (h2 << 2);
// // }
// //
// // TetgenBuilder::EdgeKey::EdgeKey(const Edge& e) {
// //   vector<Vertex*> tmp;
// //   tmp.push_back(e.initial);
// //   tmp.push_back(e.final);
// //   std::sort(tmp.begin(), tmp.end());
// //   vi = tmp[0];
// //   vf = tmp[1];
// // }
// //
// // TetgenBuilder::EdgeKey::EdgeKey(Vertex* vi, Vertex* vf) : vi(vi), vf(vf)
// {};
// //
// // bool TetgenBuilder::EdgeKey::operator==(const EdgeKey& o) const {
// //   return *vi == *o.vi && *vf == *o.vf;
// // }
// //
// // std::size_t TetgenBuilder::EdgeKey::Hash::operator()(const EdgeKey& e)
// const
// // {
// //   std::size_t h0 = Vertex::Hash()(*e.vi);
// //   std::size_t h1 = Vertex::Hash()(*e.vf);
// //   return h0 ^ (h1 << 1);
// // }
// //
// //
// //
// //
// // void assign_edges() {
// //   int size = _eMap.size();
// //   auto& edges = _mesh->edges;
// //   edges.resize(size);
// //   int i = 0;
// //   for (auto& [ek, faces] : _eMap) {
// //     Edge& e = edges[i];
// //     // Construimos el edge
// //     e.initial = ek.vi;
// //     e.final = ek.vf;
// //     e.idx = i;
// //     e.faces = faces;
// //     // Añadimos la info a la face
// //     for (int fi : e.faces) {
// //       _mesh->faces[fi].edges.push_back(e.idx);
// //     }
// //     i++;
// //   }
// // }
// //
// // void asign_faces() {
// //   for (Face& f : _mesh->faces) {
// //     auto& tmp = _fMap[f];
// //     if (tmp.size() == 0) {
// //       std::cerr << "MESH: Missing tetra from face " << f.idx << "\n";
// //       _ready = false;
// //       return;
// //     }
// //     f.initial = tmp[0];
// //     f.final = -1;
// //     if (tmp.size() > 1) f.final = tmp[1];
// //     for (int ti : tmp) {
// //       _mesh->tetras[ti].faces.push_back(f.idx);
// //     }
// //   }
// // }
// //
// // void assign_neighs_to_tetras() {
// //   for (auto& t : _mesh->tetras) {
// //     vector<int> tmp;
// //     for (auto& fi : t.faces) {
// //       Face& f = _mesh->faces[fi];
// //       // OJO! Algoritmo original guarda -1 cuando la cara es frontera
// //       if (f.initial != -1) tmp.push_back(f.initial);
// //       if (f.final != -1) tmp.push_back(f.final);
// //     }
// //     for (auto& ti : tmp) {
// //       if (ti == t.idx) continue;
// //       t.neighs.push_back(ti);
// //     }
// //   }
// //   // for (auto& face : _mesh->faces) {
// //   //   for (auto& ei : face.edges) {
// //   //     if (face.isBoundary()) {
// //   //       if (face.initial != -1)
// //   //         _mesh->edges[ei].first = face.initial;
// //   //       else
// //   //         _mesh->edges[ei].first = face.final;
// //   //     }
// //   //   }
// //   // }
// // }
// // bool ready() { return _ready; }
// // };
}  // namespace gpolylla
