#pragma once
#include <memory>
#include "GPolylla/mesh.h"

namespace GPolylla {
class PolyllaFace {
 private:
  std::shared_ptr<Tetrahedron> mesh;
};
}  // namespace GPolylla
