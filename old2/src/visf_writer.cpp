#include <fstream>
#include <iostream>

#include "gpolylla.h"

namespace gpolylla {
using namespace std;

void VisFWriter::operator()(const std::string &filename, const PolyMesh &mesh) {
  ofstream file(filename + ".visf");

  if (!file.is_open())
    cerr << "Unable to create file: " << filename << ".visf\n";

  cout << "Writing " << filename << ".visf\n";

  file << 2 << " " << 2 << "\n";

  // cantidad de puntos y puntos
  cout << "Writing vertices\n";
  file << mesh.vertices.size() << "\n";
  for (const auto &v : mesh.vertices) {
    file << v.x << " " << v.y << " " << v.z << "\n";
  }

  // cantidad de poligonos y poligonos
  cout << "Writing faces\n";
  file << mesh.faces.size() << "\n";
}

}  // namespace gpolylla
