#include <chrono>
#include <iostream>

#include "GPolylla/mesh.h"
#include "GPolylla/face.h"
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
  GPolylla::TetrahedronMesh mesh(input_mesh + ".node", input_mesh + ".face",
                                 input_mesh + ".ele");
  mesh.write_off(output_file);
  auto end = std::chrono::high_resolution_clock::now();
  mesh.display_info();
  std::cout << "Total time: "
            << std::chrono::duration_cast<std::chrono::milliseconds>(end -
                                                                     start)
                   .count()
            << "ms" << std::endl;
  polyscope::init();
  polyscope::registerSurfaceMesh(output_file, mesh.get_vertices(),
                                 mesh.get_faces());
  polyscope::show();
}
