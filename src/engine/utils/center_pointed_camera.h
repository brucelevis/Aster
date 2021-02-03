#pragma once

#include <glm/glm.hpp>

namespace Utils
{
  class CenterPointedCamera
  {
  public:
    CenterPointedCamera() = default;
    CenterPointedCamera(float r, float xAngle, float yAngle, const glm::vec2& wndSize);

    glm::mat4 GetView() const;
    glm::mat4 GetProjection() const;

    inline void SetRadius(float r)
    {
      m_Radius = r;
    }

    inline void SetXAngle(float a)
    {
      m_xAngle = a;
    }

    inline void SetYAngle(float a)
    {
      m_yAngle = a;
    }

    glm::vec3 GetWorldPosition() const;

    void Rotate(double dx, double dy);

    inline void AddRadius(double dr)
    {
      m_Radius += dr;
      if (m_Radius < 0.3f)
        m_Radius = 0.3f;
    }

  private:
    glm::vec3 m_Center;
    float m_Radius;
    float m_yAngle;
    float m_xAngle;
    glm::vec2 m_WndSize;
  };
}