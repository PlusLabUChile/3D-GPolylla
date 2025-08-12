#include "utils.h"

#include <unordered_map>
#include <unordered_set>

using namespace Polylla;
using namespace std;
using vec3 = Eigen::Vector3f;

vec3 normal(const Vertex &v0, const Vertex &v1, const Vertex &v2)
{
    return (v1 - v0).cross(v2 - v0).normalized();
}

float isOutside(const Vertex &v0, const Vertex &v1, const Vertex &v2, const Vertex &other)
{
    vec3 n = normal(v0, v1, v2);
    return (other - v0).dot(n);
}

