//
// Created by vigb9 on 01-06-2025.
//
#include <gpolylla/polylla.h>
#include <iostream>

using namespace Polylla;

int main(int argc, char* argv[]) {
  if (argc != 4) {
    std::cerr << "Usage: " << argv[0] << " <node_file> <ele_file> <output_file>" << std::endl;
  }

  TetgenReader reader;
  reader.nodeFile = argv[1];
  reader.eleFile = argv[2];
  Mesh mesh = reader.readMesh();

  PolyMesh polyMesh = CavityAlgorithm()(mesh);

  VisFWriter writer;
  writer.outputFile = argv[3];
  writer.writeMesh(polyMesh);

  std::cout << "Done" << std::endl;
  std::cout << "Created file: " << writer.outputFile << std::endl;
  return 0;
}
