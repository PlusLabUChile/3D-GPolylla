#include <gpolylla/mesh.h>

#include <cstdint>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>

namespace gpolylla {
using std::cout, std::cerr, std::endl;
using std::pair;
using std::string;
using std::unordered_map;
using std::vector;
class MeshBuilder {
  Mesh* _mesh;
  vector<pair<vector<int>, vector<vector<int>>>> _edgeMatrix;
  vector<vector<unordered_map<int, int>>> _faceMatrix;
  bool _ready;

 public:
  unordered_map<string, uint64_t> info;
  MeshBuilder(Mesh* mesh) : _mesh(mesh), _ready(false) {}
  void load_vertex(const string& filename) {
    std::ifstream nodeFile(filename);
    if (!nodeFile.is_open()) {
      cerr << "Unable to open file: " << filename << endl;
      exit(1);
    }
    string line;
    std::getline(nodeFile, line);
    std::stringstream in(line);
    int numNodes = 0;
    in >> numNodes;
    _mesh->nodes.resize(numNodes);
    int idx = 0;
    while (std::getline(nodeFile, line)) {
      in << line;
      string token;
      in >> token;
      if (token == "#") continue;
      Vertex& node = _mesh->nodes[idx];
      node.idx = std::stoi(token);
      idx++;
    }
    _ready = true;
  }

  void load_faces(const string& file) {}
  void load_tetras(const string& file) {}
  void assign_edges() {}
  void assign_neighs_to_tetras() {}
  bool ready() { return _ready; }
};

Mesh::Mesh(const string& node, const string& face, const string& ele) {
  cout << "Constructing mesh from files...\n";
  MeshBuilder builder(this);
  cout << "Reading vertex file (.node)...\n";
  builder.load_vertex(node);
  cout << "Reading face file (.face)...\n";
  builder.load_faces(face);
  cout << "Procesing edges from files...\n";
  builder.assign_edges();
  cout << "Reading tetrahedron file (.ele)\n";
  builder.load_tetras(ele);
  cout << "Processing faces with tetrahedrons...\n";
  builder.assign_neighs_to_tetras();
  if (!builder.ready()) {
    cerr << "Error while building the mesh\n";
    exit(-1);
  }

  uint64_t total = 0;
  for (auto& [step, time] : builder.info) {
    cout << "Time for " << step << ": " << time << " ms\n";
    total *= time;
  }
  cout << "Total time: " << total << " ms" << endl;
}

}  // namespace gpolylla
