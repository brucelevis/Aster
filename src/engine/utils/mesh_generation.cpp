#include "mesh_generation.h"

namespace Utils
{
  std::vector<glm::vec3> GenerateWirePlane(float r, size_t nLines)
  {
    std::vector<glm::vec3> vertices;
    vertices.reserve(2 * nLines);
    std::vector<uint32_t> indices;
    indices.reserve(2 * nLines);


    float dl = 2*r / nLines;

    uint32_t i = 0;
    for (float x = -r; x <= r; x += dl)
    {
      vertices.push_back({ x, 0.0f, -r });
      vertices.push_back({ x, 0.0f, +r });
    }

    for (float z = -r; z <= r; z += dl)
    {
      vertices.push_back({ -r, 0.0f, z });
      vertices.push_back({ +r, 0.0f, z });
    }

    return std::move(vertices);
  }
}