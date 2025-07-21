#include <gpolylla/polylla.h>

#include "utils.h"

// namespace Neighbours = Polylla::Neighbours;
// using namespace std;
// using Face = array<int, 3>;
//
// Neighbours::Mesh::Mesh(Polylla::Mesh mesh) {
//   vertices.reserve(mesh.vertices.size());
//   for (const auto& v : mesh.vertices) {
//     vertices.emplace_back(v.x(), v.y(), v.z());
//   }
//
//   auto neighs = calculateNeighbours(mesh.cells);
//
//   cells.reserve(mesh.cells.size());
//
//   const int cases[][] = {
//       {1, 2, 3},  // 0
//       {0, 2, 3},  // 1
//       {0, 1, 3},  // 2
//       {0, 1, 2}   // 3
//   };
//
//   for (int ti = 0; ti < mesh.cells.size(); ++ti) {
//     Polylla::Tetrahedron cell = mesh.cells[ti];
//     Tetrahedron newCell;
//     for (int i = 0; i < 4; ++i) {
//       newCell.vertices[i] = cell.vertices[i];
//       const int* config = cases[i];
//       Face f = {cell.vertices[config[0]], cell.vertices[config[1]],
//                 cell.vertices[config[2]]};
//       ranges::sort(f);
//       auto n = neighs.at(f);
//       newCell.neighs[i] = n[0];
//       if (n[0] == ti) {
//         newCell.neighs[i] = n[1];
//       }
//     }
//     cells.push_back(newCell);
//   }
// }