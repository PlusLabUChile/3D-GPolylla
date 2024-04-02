#include "GPolylla/face.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <iterator>
#include <numeric>
#include <set>
#include <unordered_map>
#include <unordered_set>

#include "GPolylla/mesh.h"

namespace GPolylla {
using std::shared_ptr;
using std::vector;

void calculate_edges_length(TetrahedronMesh* mesh) {
  for (auto& edge : mesh->edges) {
    auto v1 = mesh->nodes[edge.vi];
    auto v2 = mesh->nodes[edge.vf];
    double dx = v1.x - v2.x;
    double dy = v1.y - v2.y;
    double dz = v1.z - v2.z;
    edge.length = std::sqrt(dx * dx + dy * dy + dz * dz);
  }
}

PolyllaFace::PolyllaFace(const shared_ptr<TetrahedronMesh>& mesh)
    : n_barries_faces(0), polyhedra_with_barries(0) {
  this->mesh = mesh;
  calculate_max_incircle_faces();
  calculate_seed_tetrahedrons();
  calculate_frontier_faces();

  visited_tetra.resize(mesh->num_tetrahedrons(), false);
  seed_tetra_in_repair.resize(mesh->num_tetrahedrons(), false);

  for (int ti : seed_tetra) {
    vector<int> polyhedron;
    vector<int> polyhedron_tetras;
    depth_first_search(&polyhedron, &polyhedron_tetras, ti);
    // depth_first_search(&polyhedron, &polyhedron_tetras, ti);
    // check if polyhedron has barrier faces
    int barrier_faces = count_barrier_faces(polyhedron);
    if (barrier_faces > 0) {
      for (int tp : polyhedron_tetras) {
        // we need to repair the polyhedron,
        // so we mark the tetrahedrons as not visited
        visited_tetra[tp] = false;
      }
      // generate a list of the barrier faces tips
      vector<int> barrier_face_tips;
      detect_barrier_face_tips(polyhedron, &barrier_face_tips);
      // sent the polyhedron to repair
      repair_phase(polyhedron, barrier_face_tips);
    } else {
      Polyhedron poly;
      poly.tetrahedrons = polyhedron_tetras;
      poly.faces = polyhedron;
      polyhedral_mesh.push_back(poly);
    }
  }
}

void PolyllaFace::calculate_max_incircle_faces() {
  std::cout << "Calculating max incircle faces..." << std::endl;
  calculate_edges_length(mesh.get());
  vector<double> face_radious;
  for (auto& face : mesh->faces) {
    double length_edge_a = mesh->edges[face.edges[0]].length;
    double length_edge_b = mesh->edges[face.edges[1]].length;
    double length_edge_c = mesh->edges[face.edges[2]].length;
    double semiperimeter = (length_edge_a + length_edge_b + length_edge_c) / 2;
    double radious = (semiperimeter - length_edge_a) *
                     (semiperimeter - length_edge_b) *
                     (semiperimeter - length_edge_c) / semiperimeter;
    face_radious.push_back(radious);
  }

  for (auto& tetra : mesh->tetras) {
    double a0 = face_radious[tetra.faces[0]];
    double a1 = face_radious[tetra.faces[1]];
    double a2 = face_radious[tetra.faces[2]];
    double a3 = face_radious[tetra.faces[3]];
    double max_face = std::max(std::max(a0, a1), std::max(a2, a3));
    if (max_face == a0) {
      longest_faces.push_back(0);
    } else if (max_face == a1) {
      longest_faces.push_back(1);
    } else if (max_face == a2) {
      longest_faces.push_back(2);
    } else if (max_face == a3) {
      longest_faces.push_back(3);
    } else {
      std::cerr << "Error in function calculate_max_incircle_faces"
                << std::endl;
    }
  }
}

void PolyllaFace::calculate_seed_tetrahedrons() {
  std::cout << "Calculating seed tetrahedrons..." << std::endl;
  for (int fi = 0; fi < mesh->num_faces(); fi++) {
    int n1 = mesh->faces[fi].ni;
    int n2 = mesh->faces[fi].nf;

    if (n1 == -1 && mesh->get_tetra(n2).faces[longest_faces[n2]] == fi) {
      seed_tetra.push_back(n2);
    } else if (n2 == -1 && mesh->get_tetra(n1).faces[longest_faces[n1]] == fi) {
      seed_tetra.push_back(n1);
    } else {
      // fix -1 values
      n1 = (n1 + longest_faces.size()) % longest_faces.size();
      n2 = (n2 + longest_faces.size()) % longest_faces.size();
      int longest_face_n1 = mesh->get_tetra(n1).faces[longest_faces[n1]];
      int longest_face_n2 = mesh->get_tetra(n2).faces[longest_faces[n2]];
      if (fi == longest_face_n1 && fi == longest_face_n2) {
        seed_tetra.push_back(n1);
      }
    }
  }
}

void PolyllaFace::calculate_frontier_faces() {
  std::cout << "Calculating frontier faces..." << std::endl;
  for (int fi = 0; fi < mesh->num_faces(); fi++) {
    int n1 = mesh->get_face(fi).ni;
    int n2 = mesh->get_face(fi).nf;
    frontier_faces.reserve(mesh->num_faces());
    if (n1 == -1 || n2 == -1) {
      frontier_faces.push_back(true);
    } else {
      int longest_face_n1 = mesh->get_tetra(n1).faces[longest_faces[n1]];
      int longest_face_n2 = mesh->get_tetra(n2).faces[longest_faces[n2]];
      frontier_faces.push_back(fi != longest_face_n1 && fi != longest_face_n2);
    }
  }
}

void PolyllaFace::depth_first_search(vector<int>* polyhedron,
                                     vector<int>* polyhedron_tetras,
                                     int tetra) {
  visited_tetra[tetra] = true;
  polyhedron_tetras->push_back(tetra);

  for (int i = 0; i < 4; i++) {
    int fi = mesh->get_tetra(tetra).faces[i];
    const auto& neighs = mesh->get_tetra(tetra).neighs;
    if (fi != -1) {
      if (frontier_faces[fi]) {
        polyhedron->push_back(fi);
      } else {
        int next = neighs[i];
        if (!visited_tetra[next]) {
          depth_first_search(polyhedron, polyhedron_tetras, next);
        }
      }
    }
  }
}

int PolyllaFace::count_barrier_faces(const std::vector<int>& polyhedron) {
  std::unordered_map<int, int> counter;
  for (int fi : polyhedron) {
    counter[fi]++;
  }
  int repeated =
      std::reduce(counter.begin(), counter.end(), std::pair(0, 0),
                  [](auto& acc, auto& pair) {
                    return std::pair(0, acc.second + (pair.second > 1));
                  })
          .second;

  if (repeated > 0) {
    n_barries_faces += repeated;
    polyhedra_with_barries++;
  }
  return repeated;
}

void PolyllaFace::detect_barrier_face_tips(
    const std::vector<int>& terminal_face,
    std::vector<int>* barrier_face_tips) {
  std::unordered_map<int, int> counter;
  for (int fi : terminal_face) {
    counter[fi]++;
  }

  // List of all repeated faces
  std::vector<int> barrier_faces;
  for (auto& pair : counter) {
    if (pair.second > 1) {
      barrier_faces.push_back(pair.first);
    }
  }

  // List all edges of the barrier faces
  std::unordered_set<int> possible_tips;
  for (int fi : barrier_faces) {
    for (int ei : mesh->get_face(fi).edges) {
      possible_tips.insert(ei);
    }
  }

  std::set<int> tf_set(terminal_face.begin(), terminal_face.end());
  for (int ei : possible_tips) {
    auto& face_of_edges = mesh->get_edge(ei).faces;
    std::set<int> foe_set(face_of_edges.begin(), face_of_edges.end());
    std::set<int> inter;
    std::set_intersection(foe_set.begin(), foe_set.end(), tf_set.begin(),
                          tf_set.end(), std::inserter(inter, inter.begin()));
    int l1 = inter.size();
    int l2 = terminal_face.size();
    if (l2 - l1 == l2 - 1) {
      barrier_face_tips->push_back(ei);
    }
  }
}
void PolyllaFace::repair_phase(const std::vector<int>& polyhedron,
                               const std::vector<int>& barrier_face_tips) {
  std::vector<int> tetra_list;
  int barrier_face = -1;
  for (int ei : barrier_face_tips) {
    // search polyhedron that contains the edge e
    for (int fi : polyhedron) {
      auto& face_edges = mesh->get_face(fi).edges;
      if (std::find(face_edges.begin(), face_edges.end(), ei) !=
          face_edges.end()) {
        barrier_face = fi;
        break;
      }
    }
    auto& faces_of_barrier_face_tip = mesh->get_edge(ei).faces;
    int num_internal_faces = faces_of_barrier_face_tip.size() - 1;
    int adv = num_internal_faces % 2 == 0 ? num_internal_faces / 2 - 1
                                          : num_internal_faces / 2;
    // because the first face is the barrierFace, advance to the next
    // internal-face
    auto itpos = std::find(faces_of_barrier_face_tip.begin(),
                           faces_of_barrier_face_tip.end(), barrier_face);
    int pos = std::distance(faces_of_barrier_face_tip.begin(), ++itpos);
    int middle_face =
        faces_of_barrier_face_tip[(pos + adv) % num_internal_faces];

    // if there no advance, the middle face is the barrierFace and there is not
    // repair
    if (middle_face == barrier_face) {
      std::cerr << "middle_Face == faces_of_barrierFaceTip" << std::endl;
      exit(1);
    }

    // convert the middle internalface into a frontier-face
    frontier_faces[middle_face] = true;

    // store adjacent tetrahedrons to the sub seed list
    int t1 = mesh->get_face(barrier_face).ni;
    int t2 = mesh->get_face(barrier_face).nf;
    tetra_list.push_back(t1);
    tetra_list.push_back(t2);

    // mark to be use in the seed_tetra
    seed_tetra_in_repair[t1] = true;
    seed_tetra_in_repair[t2] = true;
  }

  // while tetra_list is not empty
  while (!tetra_list.empty()) {
    int tetra_curr = tetra_list.back();
    tetra_list.pop_back();
    if (seed_tetra_in_repair[tetra_curr]) {
      seed_tetra_in_repair[tetra_curr] = false;
      vector<int> new_polyhedron;
      vector<int> new_polyhedron_tetras;
      depth_first_search(&new_polyhedron, &new_polyhedron_tetras, tetra_curr);
      Polyhedron poly;
      poly.faces = new_polyhedron;
      poly.tetrahedrons = new_polyhedron_tetras;
      poly.was_repaired = true;
      polyhedral_mesh.push_back(poly);
    }
  }
}

Polyhedron& PolyllaFace::get_polyhedron(int id) {
  return polyhedral_mesh.at(id);
}

int PolyllaFace::num_polyhedra() const { return polyhedral_mesh.size(); }

}  // namespace GPolylla
