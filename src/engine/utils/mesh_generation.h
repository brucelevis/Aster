#pragma once

#include <tuple>

#include <glm/glm.hpp>
#include <vector>

namespace Utils
{
  std::vector<glm::vec3> GenerateWirePlane(float r, size_t nLines);
}