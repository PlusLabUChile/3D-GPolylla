#include <gpolylla/polylla.h>

#include <fstream>

#include "utils.h"

using namespace Polylla;
using namespace std;

struct Face {
  array<int, 3> vertices;
  array<int, 2> tetras;
};

void VisFWriter::writeMesh(PolyMesh mesh) {
  /*
  ofstream file(meshName + ".visf");
  if (!file.is_open()) {
    throw runtime_error("Unable to create file: " + meshName + ".visf");
  }

  // primer valor -> formato del archivo
  // 0 -> big endian
  // 1 -> little endian
  // 2 -> ascii

  // segundo valor -> tipo de malla
  // 0 -> nube de puntos
  // 1 -> malla de poligonos
  // 2 -> malla de poliedros
  file << 2 << " " << 2 << endl;

  // cantidad de puntos y puntos
  file << mesh.vertices.size() << endl;
  for (const auto &v : mesh.vertices) {
    file << v.x << " " << v.y << " " << v.z << endl;
  }

  // cantidad de poligonos y poligonos
  auto neighs = calculateNeighbours(mesh.tetras);

  file << mesh.faces.size() << endl;
  for (const auto &f : mesh.faces) {
    file << f.vertices.size();
    for (int vi : f.vertices) {
      file << " " << vi;
    }
    file << endl;
  }

  // relacion de vecindad entre poligonos
  file << 0 << endl;
  // numero de poliedros y poliedros (basado en poligonos)
  file << mesh.cells.size() << endl;
  // polyhedra are saved based on faces
  for (const auto &p : mesh.cells) {
    file << p.faces.size();
    for (int fi : p.faces) {
      file << " " << fi;
    }
    file << endl;
  }
  cout << "Done" << endl;
  */
}
