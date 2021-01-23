#pragma once

#include <glm/glm.hpp>

namespace App
{
  class Camera
  {
  public:
    Camera() = default;
    Camera(const glm::vec3 position, const glm::vec2& wndSize);

    glm::mat4 GetView() const;
    glm::mat4 GetProjection() const;

    inline void SetPosition(const glm::vec3& position)
    {
      m_Position = position;
    }

  private:
    glm::vec3 m_Position;
    glm::vec2 m_WndSize;
  };
}