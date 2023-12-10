#include <array>
#include <chrono>
#include <iostream>
#include <memory>

#include "GPolylla/face.h"
#include "GPolylla/mesh.h"
#include "polyscope/polyscope.h"
#include "polyscope/surface_mesh.h"
#include "polyscope/utilities.h"
#include "polyscope/volume_mesh.h"

int main(int argc, char* argv[]) {
  using std::array;
  using std::cout;
  using std::endl;
  using std::string;
  using std::vector;
  if (argc != 3) {
    std::cout << "Usage: " << argv[0] << " <input_mesh> <output_file>"
              << std::endl;
    return 1;
  }
  string input_mesh(argv[1]);
  string output_file(argv[2]);
  auto start = std::chrono::high_resolution_clock::now();
  auto mesh = std::make_shared<GPolylla::TetrahedronMesh>(
      input_mesh + ".node", input_mesh + ".face", input_mesh + ".ele");
  GPolylla::PolyllaFace pmesh(mesh);
  auto end = std::chrono::high_resolution_clock::now();
  auto delta1 = std::chrono::duration_cast<std::chrono::seconds>(end - start);
  cout << "Total time: " << delta1.count() << " secs" << endl;
  mesh->display_info();
  vector<array<double, 3>> pol_colors(pmesh.num_polyhedra());
  for (int i = 0; i < pmesh.num_polyhedra(); i++) {
    pol_colors[i] = {polyscope::randomUnit(), polyscope::randomUnit(),
                     polyscope::randomUnit()};
  }
  vector<array<double, 3>> cell_colors(mesh->num_tetrahedrons());
  for (int i = 0; i < pmesh.num_polyhedra(); i++) {
    auto& poly = pmesh.get_polyhedron(i);
    for (int ti : poly.tetras) {
      cell_colors[ti] = pol_colors[i];
    }
  }
  polyscope::init();
  polyscope::registerTetMesh(output_file, mesh->get_vertices(),
                             mesh->get_tetrahedrons());
  // polyscope::registerSurfaceMesh(output_file, mesh->get_vertices(),
  //                                mesh->get_faces());
  // polyscope::getSurfaceMesh(output_file)
  //     ->addFaceColorQuantity("polylla face", face_colors);
  polyscope::getVolumeMesh(output_file)->addCellColorQuantity("polylla face",
                                                            cell_colors);
  polyscope::show();
}
