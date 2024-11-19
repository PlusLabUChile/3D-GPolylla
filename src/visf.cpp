#include <gpolylla/io.h>

#include <fstream>
#include <iostream>
#include <unordered_map>
#include <vector>

namespace gpolylla {
using std::cerr, std::endl;
using std::ofstream;
using std::string;
using std::vector;

void VisFWriter::setBasename(const string& basename) {
  this->basename = basename;
}

void VisFWriter::write(const PolyMesh& mesh) {
  ofstream file(basename + ".visf");
  if (!file.is_open()) {
    cerr << "Unable to create file: " << basename << ".visf" << endl;
  }

  file << 2 << " " << 2 << endl;
  file << mesh.vertices.size() << endl;
  for (const auto& v : mesh.vertices) {
    file << v.x << " " << v.y << " " << v.z << endl;
  }

  file << mesh.faces.size() << endl;
  for (const auto& f : mesh.faces) {
    file << f.vertices.size();
    for (int vi : f.vertices) {
      file << " " << vi;
    }
    file << endl;
  }

  file << mesh.cells.size() << endl;
  // polyhedra are saved based on faces
  for (const auto& p : mesh.cells) {
    file << p.faces.size();
    for (int vi : p.faces) {
      file << " " << vi;
    }
    file << endl;
  }
}

}  // namespace gpolylla
