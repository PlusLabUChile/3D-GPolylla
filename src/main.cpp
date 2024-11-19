#ifndef GPOLYLLA_LIB
#include <array>
#include <chrono>
#include <iostream>
#include <memory>
#include <string>

#include "gpolylla/algo.h"
#include "gpolylla/io.h"
#include "gpolylla/mesh.h"
#include "gpolylla/utils.h"
#include "polyscope/point_cloud.h"
#include "polyscope/polyscope.h"
#include "polyscope/utilities.h"
#include "polyscope/volume_mesh.h"

int main(int argc, char* argv[]) {
  using std::array;
  using std::cout;
  using std::string;
  using std::vector;
  if (argc != 3) {
    std::cout << "Usage: " << argv[0] << " <input_mesh> <output_file>"
              << std::endl;
    return 1;
  }
  string inputMesh(argv[1]);
  string outputFile(argv[2]);
  gpolylla::TetgenReader reader;
  gpolylla::CavityAlgorithm algo;
  gpolylla::TetraMesh inMesh;
  gpolylla::PolyMesh outMesh;

  reader.setBasename(inputMesh);
  auto t0 = std::chrono::high_resolution_clock::now();
  inMesh = reader.build();
  auto t1 = std::chrono::high_resolution_clock::now();
  outMesh = algo(inMesh);
  auto t2 = std::chrono::high_resolution_clock::now();

  cout << "Reading mesh from files: "
       << std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0)
       << "\n";
  cout << "Algorythm proccessing: "
       << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1)
       << "\n";
  cout << "Total: "
       << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t0)
       << "\n";

  cout << "Final mesh info:\n";
  cout << "Number of polyhedrons: " << outMesh.cells.size() << "\n";
  cout << "Number of mesh faces: " << outMesh.faces.size() << "\n";
  cout << "Number of mesh edges: " << outMesh.edges.size() << "\n";

  vector<array<double, 3>> colors(inMesh.tetras.size());
  for (const auto& poly : outMesh.cells) {
    array<double, 3> c = {polyscope::randomUnit(), polyscope::randomUnit(),
                          polyscope::randomUnit()};
    for (int ti : poly.tetras) {
      colors[ti] = c;
    }
  }

  polyscope::init();
  // std::vector<std::array<double, 3>> points;
  // for (auto& ti : outMesh.tetras) {
  //   auto center = gpolylla::circumsphere(
  //       outMesh.vertices[ti.vertices[0]], outMesh.vertices[ti.vertices[1]],
  //       outMesh.vertices[ti.vertices[2]], outMesh.vertices[ti.vertices[3]]);
  //   points.push_back({center.x(), center.y(), center.z()});
  // }
  // auto* sphere = polyscope::registerPointCloud("circumspheres", points);
  // sphere->setEnabled(false);

  auto* mainMesh =
      polyscope::registerTetMesh(outputFile, inMesh.vertices, inMesh.tetras);
  // polyscope::registerSurfaceMesh(output_file, mesh->get_vertices(),
  //                                mesh->get_faces());
  // polyscope::getSurfaceMesh(output_file)
  //     ->addFaceColorQuantity("polylla face", face_colors);
  mainMesh->addCellColorQuantity("polylla face", colors);
  for (auto poly : outMesh.cells) {
    vector<gpolylla::Vertex> pVert;
    vector<array<int, 4>> pTetras;
    for (int vi : poly.vertices) pVert.push_back(outMesh.vertices[vi]);
    for (int ti : poly.tetras) {
      array<int, 4> temp;
      for (int i = 0; i < 4; i++) {
        int target = inMesh.tetras[ti].vertices[i];
        // std::cout << inMesh.tetras[ti] << std::endl;
        for (int vi = 0; vi < pVert.size(); vi++) {
          if (pVert[vi].idx == target) temp[i] = vi;
        }
      }
      pTetras.push_back(temp);
    }
    auto* mesh =
        polyscope::registerTetMesh(std::to_string(poly.idx), pVert, pTetras);
    mesh->setEnabled(false);
  }

  polyscope::show();
}

#endif  // !GPOLYLLA_LIB
