#pragma once

#include <glm/glm.hpp>

namespace Math
{
  glm::mat4 Perspective(float fov, float aspect, float zNear, float zFar);

  glm::mat4 LookAt(const glm::vec3& at, const glm::vec3& from, const glm::vec3& up);

  float Radians(float degree);
}