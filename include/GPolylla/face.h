#pragma once
#include <memory>
#include <vector>

#include "GPolylla/mesh.h"

namespace GPolylla {
class PolyllaFace {
 private:
  std::shared_ptr<TetrahedronMesh> mesh;
  int n_barries_faces;
  int polyhedra_with_barries;
  std::vector<int> longest_faces;
  std::vector<int> seed_tetra;
  std::vector<bool> frontier_faces;
  std::vector<bool> visited_tetra;
  std::vector<bool> seed_tetra_in_repair;
  std::vector<Polyhedron> polyhedral_mesh;

 private:
  void calculate_max_incircle_faces();
  void calculate_seed_tetrahedrons();
  void calculate_frontier_faces();
  int count_barrier_faces(const std::vector<int>& polyhedron);
  void detect_barrier_face_tips(const std::vector<int>& polyhedron,
                                std::vector<int>* barrier_face_tips);
  void repair_phase(const std::vector<int>& polyhedron,
                    const std::vector<int>& barrier_face_tips);
  void depth_first_search(std::vector<int>* polyhedra,

                          std::vector<int>* poly_tetras, int tetra);

 public:
  explicit PolyllaFace(const std::shared_ptr<TetrahedronMesh>& mesh);
  Polyhedron& get_polyhedron(int id);
  int num_polyhedra() const;
};
}  // namespace GPolylla
