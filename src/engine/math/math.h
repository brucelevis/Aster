#pragma once

#include <glm/glm.hpp>
#include <math.h>

namespace Math
{
  inline glm::mat4 Perspective(float fov, float aspect, float zNear, float zFar)
  {
    const float tanA = std::tanf(fov / 2.0);

    glm::mat4 m(0);
    m[0][0] = 1 / (aspect * tanA);
    m[1][1] = 1 / tanA;
    m[2][2] = zFar / (zFar - zNear);
    m[2][3] = 1.0f;
    m[3][2] = -(zFar * zNear) / (zFar - zNear);

    return m;
  }
}