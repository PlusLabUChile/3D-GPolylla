// #include <gpolylla/io/reader.h>
//
// #include <fstream>
// #include <sstream>
//
// namespace gpolylla {
// using std::cout, std::cerr, std::endl;
// using std::ostream;
// using std::string;
// using std::unordered_map;
// using std::vector;
//
// TetgenBuilder::TetgenBuilder(const std::string& basename)
//     : _basename(basename) {}
// TetraMesh TetgenBuilder::concrete_build() {
//   cout << "Constructing mesh from files...\n";
//   cout << "Reading vertex file (.node)...\n";
//   load_node(_basename + ".node");
//   cout << "Reading face file (.face)...\n";
//   load_face(_basename + ".face");
//   // cout << "Procesing edges from files...\n";
//   // builder.assign_edges();
//   cout << "Reading tetrahedron file (.ele)...\n";
//   load_ele(_basename + ".ele");
//   cout << "Applying connectivity to the mesh...\n";
//   apply_connectivity();
//   // cout << "Processing faces with tetrahedrons...\n";
//   // builder.assign_neighs_to_tetras();
//   // if (!builder.ready()) {
//   //   cerr << "Error while building the mesh\n";
//   //   exit(-1);
//   // }
//   return _mesh;
// }
//
// TetgenBuilder::FaceKey::FaceKey(const Face& f) {
//   vector<Vertex*> tmp;
//   tmp = f.vertices;
//   std::sort(tmp.begin(), tmp.end());
//   v0 = tmp[0];
//   v1 = tmp[1];
//   v2 = tmp[2];
// }
//
// TetgenBuilder::FaceKey::FaceKey(Vertex* v0, Vertex* v1, Vertex* v2)
//     : v0(v0), v1(v1), v2(v2) {}
//
// bool TetgenBuilder::FaceKey::operator==(const FaceKey& o) const {
//   return v0 == o.v0 && v1 == o.v1 && v2 == o.v2;
// }
//
// std::size_t TetgenBuilder::FaceKey::Hash::operator()(const FaceKey& f) const {
//   std::size_t h0 = Vertex::Hash()(*f.v0);
//   std::size_t h1 = Vertex::Hash()(*f.v1);
//   std::size_t h2 = Vertex::Hash()(*f.v2);
//   return h0 ^ (h1 << 1) ^ (h2 << 2);
// }
//
// TetgenBuilder::EdgeKey::EdgeKey(const Edge& e) {
//   vector<Vertex*> tmp;
//   tmp.push_back(e.initial);
//   tmp.push_back(e.final);
//   std::sort(tmp.begin(), tmp.end());
//   vi = tmp[0];
//   vf = tmp[1];
// }
//
// TetgenBuilder::EdgeKey::EdgeKey(Vertex* vi, Vertex* vf) : vi(vi), vf(vf) {};
//
// bool TetgenBuilder::EdgeKey::operator==(const EdgeKey& o) const {
//   return *vi == *o.vi && *vf == *o.vf;
// }
//
// std::size_t TetgenBuilder::EdgeKey::Hash::operator()(const EdgeKey& e) const {
//   std::size_t h0 = Vertex::Hash()(*e.vi);
//   std::size_t h1 = Vertex::Hash()(*e.vf);
//   return h0 ^ (h1 << 1);
// }
//
// bool TetgenBuilder::load_node(const string& filename) {
//   std::ifstream nodeFile(filename);
//   if (!nodeFile.is_open()) {
//     cerr << "Unable to open file: " << filename << endl;
//     return false;
//   }
//   string line;
//   std::getline(nodeFile, line);
//   std::stringstream in(line);
//   int numNodes = 0;
//   in >> numNodes;
//   _mesh.nodes.reserve(numNodes);
//   while (std::getline(nodeFile, line)) {
//     in.str(line);
//     string token;
//     in >> token;
//     if (token == "#") continue;
//     // Vertex& node = _mesh.nodes[idx];
//     int idx = std::stoi(token);
//     float x, y, z;
//     in >> x >> y >> z;
//     _mesh.nodes.push_back(Vertex{idx, x, y, z});
//     in.clear();
//   }
// }
//
// void load_faces(const string& filename) {
//   std::ifstream faceFile(filename);
//   if (!faceFile.is_open()) {
//     cerr << "Unable to open file: " << filename << endl;
//     _ready = false;
//     return;
//   }
//   string line;
//   std::getline(faceFile, line);
//   std::stringstream in(line);
//   int numFaces = 0;
//   in >> numFaces;
//   _mesh->faces.resize(numFaces);
//   int idx = 0;
//   while (std::getline(faceFile, line)) {
//     in.str(line);
//     string token;
//     in >> token;
//     if (token == "#") continue;
//     Face& face = _mesh->faces[idx];
//     face.idx = std::stoi(token);
//     face.vertices.resize(3);
//     in >> face.vertices[0] >> face.vertices[1] >> face.vertices[2];
//     idx++;
//     in.clear();
//     // add the edges to the edge map
//     vector<int> v;
//     v = face.vertices;
//     std::sort(v.begin(), v.end());
//     _eMap[{v[0], v[1]}].push_back(face.idx);
//     _eMap[{v[0], v[2]}].push_back(face.idx);
//     _eMap[{v[1], v[2]}].push_back(face.idx);
//   }
// }
//
// void load_tetras(const string& filename) {
//   std::ifstream eleFile(filename);
//   if (!eleFile.is_open()) {
//     cerr << "Unable to open file: " << filename << endl;
//     _ready = false;
//     return;
//   }
//   string line;
//   std::getline(eleFile, line);
//   std::stringstream in(line);
//   int numTetras = 0, sizeTetras = 0;
//   in >> numTetras >> sizeTetras;
//   _mesh->tetras.resize(numTetras);
//   int idx = 0;
//   while (std::getline(eleFile, line)) {
//     in.str(line);
//     string token;
//     in >> token;
//     if (token == "#") continue;
//     Tetrahedron& t = _mesh->tetras[idx];
//     t.idx = std::stoi(token);
//     t.vertices.resize(sizeTetras);
//     for (auto& v : t.vertices) in >> v;
//     idx++;
//     in.clear();
//     // add the tetra to the faces map
//     vector<int> v;
//     v = t.vertices;
//     std::sort(v.begin(), v.end());
//     _fMap[{v[0], v[1], v[2]}].push_back(t.idx);
//     _fMap[{v[0], v[1], v[3]}].push_back(t.idx);
//     _fMap[{v[0], v[2], v[3]}].push_back(t.idx);
//     _fMap[{v[1], v[2], v[3]}].push_back(t.idx);
//   }
//
//   asign_faces();
//
//   // for (auto& [face, tetras] : _fMap) {
//   //   std::cout << "(" << face.v0 << " " << face.v1 << " " << face.v2 << ") :
//   //   "; for (auto& v : tetras) {
//   //     std::cout << v << " ";
//   //   }
//   //   std::cout << "\n";
//   // }
// }
//
// void assign_edges() {
//   int size = _eMap.size();
//   auto& edges = _mesh->edges;
//   edges.resize(size);
//   int i = 0;
//   for (auto& [ek, faces] : _eMap) {
//     Edge& e = edges[i];
//     // Construimos el edge
//     e.initial = ek.vi;
//     e.final = ek.vf;
//     e.idx = i;
//     e.faces = faces;
//     // Añadimos la info a la face
//     for (int fi : e.faces) {
//       _mesh->faces[fi].edges.push_back(e.idx);
//     }
//     i++;
//   }
// }
//
// void asign_faces() {
//   for (Face& f : _mesh->faces) {
//     auto& tmp = _fMap[f];
//     if (tmp.size() == 0) {
//       std::cerr << "MESH: Missing tetra from face " << f.idx << "\n";
//       _ready = false;
//       return;
//     }
//     f.initial = tmp[0];
//     f.final = -1;
//     if (tmp.size() > 1) f.final = tmp[1];
//     for (int ti : tmp) {
//       _mesh->tetras[ti].faces.push_back(f.idx);
//     }
//   }
// }
//
// void assign_neighs_to_tetras() {
//   for (auto& t : _mesh->tetras) {
//     vector<int> tmp;
//     for (auto& fi : t.faces) {
//       Face& f = _mesh->faces[fi];
//       // OJO! Algoritmo original guarda -1 cuando la cara es frontera
//       if (f.initial != -1) tmp.push_back(f.initial);
//       if (f.final != -1) tmp.push_back(f.final);
//     }
//     for (auto& ti : tmp) {
//       if (ti == t.idx) continue;
//       t.neighs.push_back(ti);
//     }
//   }
//   // for (auto& face : _mesh->faces) {
//   //   for (auto& ei : face.edges) {
//   //     if (face.isBoundary()) {
//   //       if (face.initial != -1)
//   //         _mesh->edges[ei].first = face.initial;
//   //       else
//   //         _mesh->edges[ei].first = face.final;
//   //     }
//   //   }
//   // }
// }
// bool ready() { return _ready; }
// };
// }  // namespace gpolylla
