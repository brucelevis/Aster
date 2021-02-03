#pragma once

#include <tuple>

#include <glm/glm.hpp>
#include <vector>

namespace Utils
{
  std::vector<glm::vec3> GenerateWirePlane(float r, size_t nLines);

  std::tuple<std::vector<glm::vec3>, std::vector<uint32_t>> GenerateSphere(int splitFacesNTimes, float radius);
}