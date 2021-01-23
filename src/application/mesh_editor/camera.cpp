#include "camera.h"

#include <engine/math/math.h>

#include <glm/gtc/matrix_transform.hpp>

namespace App
{
  Camera::Camera(const glm::vec3 position, const glm::vec2& wndSize)
    : m_Position(position)
    , m_WndSize(wndSize)
  {
  }

  glm::mat4 Camera::GetProjection() const
  {
    return Math::Perspective(90.0f, m_WndSize.x / m_WndSize.y, 0.1f, 100.0f);
  }

  glm::mat4 Camera::GetView() const
  {
    return Math::LookAt({ 0.0f, 0.0f, 0.0f }, m_Position, { 0.0f, 1.0f, 0.0f });
  }
}