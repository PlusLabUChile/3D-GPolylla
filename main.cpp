#include <chrono>
#include <iostream>
#include <memory>

#include "GPolylla/face.h"
#include "GPolylla/mesh.h"
#include "polyscope/polyscope.h"
#include "polyscope/surface_mesh.h"

int main(int argc, char *argv[]) {
  if (argc != 3) {
    std::cout << "Usage: " << argv[0] << " <input_mesh> <output_file>"
              << std::endl;
    return 1;
  }
  std::string input_mesh(argv[1]);
  std::string output_file(argv[2]);
  auto start = std::chrono::high_resolution_clock::now();
  auto mesh = std::make_shared<GPolylla::TetrahedronMesh>(
      input_mesh + ".node", input_mesh + ".face", input_mesh + ".ele");
  auto end = std::chrono::high_resolution_clock::now();
  auto delta1 =
      std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
  std::cout << "Total construction time: " << delta1.count() << "ms"
            << std::endl;
  // mesh->write_off(output_file);
  start = std::chrono::high_resolution_clock::now();
  GPolylla::PolyllaFace pmesh(mesh);
  end = std::chrono::high_resolution_clock::now();
  auto delta2 =
      std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
  std::cout << "Total Polylla time: " << delta2.count() << "ms" << std::endl;
  mesh->display_info();
  polyscope::init();
  polyscope::registerSurfaceMesh(output_file, mesh->get_vertices(),
                                 mesh->get_faces());
  polyscope::show();
}
