#ifndef GPOLYLLA_PARTITION_H
#define GPOLYLLA_PARTITION_H
#include "polylla.h"
namespace Polylla
{
// Cell IDs may be non-contiguous; output IDs are canonical by first tetrahedron.
PolyMesh fromMembership(const Mesh &mesh, const std::vector<int> &membership);
// Throws on incomplete coverage, disconnected cells, or incorrect boundaries.
void validatePartition(const PolyMesh &mesh);
} // namespace Polylla
#endif
