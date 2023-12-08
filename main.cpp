#include <mesh.h>

#include <iostream>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cout << "Usage: " << argv[0] << " <mesh_name>" << std::endl;
    return 1;
  }
  std::string mesh_name(argv[1]);
  mesh_name += ".1";
  GPolylla::TetrahedronMesh mesh("data/" + mesh_name + ".node",
                                 "data/" + mesh_name + ".face",
                                 "data/" + mesh_name + ".ele",
                                 "data/" + mesh_name + ".edge");
  mesh.display_info();
}
