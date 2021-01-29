#include "camera.h"

#include <engine/math/math.h>

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>

namespace Editor
{
  Camera::Camera(float r, float xAngle, float yAngle, const glm::vec2& wndSize)
    : m_Center({ 0.0f, 0.0f, 0.0f })
    , m_Radius(r)
    , m_xAngle(xAngle)
    , m_yAngle(yAngle)
    , m_WndSize(wndSize)
  {
  }

  glm::mat4 Camera::GetProjection() const
  {
    return Math::Perspective(90.0f, m_WndSize.x / m_WndSize.y, 0.1f, 100.0f);
  }

  glm::mat4 Camera::GetView() const
  {
    const glm::vec3 pos = GetWorldPosition();
    return Math::LookAt(m_Center, pos, { 0.0f, 1.0f, 0.0f });
  }

  glm::vec3 Camera::GetWorldPosition() const
  {
    const float dy = Math::Radians(m_yAngle);
    const float dx = Math::Radians(m_xAngle);

    const float y = m_Radius * std::cos(dy);
    const float r = m_Radius * std::sin(dy);
    const float x = r * std::cos(dx);
    const float z = r * std::sin(dx);

    return glm::vec3{ x,y,z };
  }

  void Camera::Rotate(double dx, double dy)
  {
    m_xAngle -= dx * 0.3f;
    m_yAngle -= dy * 0.3f;

    if (m_xAngle < -360.0f || m_xAngle > 360.0f)
      m_xAngle = 0.0f;

    m_yAngle = std::clamp(m_yAngle, 1.0f, 179.0f);
  }
}