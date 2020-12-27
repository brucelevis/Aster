#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"

enum OrientationVectorType
{
  Forward,
  Right,
  Up
};

struct Transform
{
  Transform();

  Transform* Parent;

  glm::vec3 LocalPosition = { 0.0f, 0.0f, 0.0f };
  glm::quat LocalRotation = {1.0f, 0.0f, 0.0f, 0.0f};
  glm::vec3 LocalScale = { 1.0f, 1.0f, 1.0f };

  void AttachTo(Transform* Parent);

  glm::vec3 GetWorldPosition() const;
  glm::quat GetWorldRotation() const;
  glm::vec3 GetWorldScale() const;

  glm::mat4 GetTransformationMatrix() const;
  glm::mat4 GetTransformationMatrixWithoutScale() const;
  glm::mat4 GetCameraTransformationMatrixWithoutScale() const;

  glm::vec3 GetOrientationVector(OrientationVectorType type) const;
  
  inline glm::vec3 Transform::GetForwardVector() const
  {
    return GetOrientationVector(Forward);
  }

  inline glm::vec3 Transform::GetUpVector() const
  {
    return GetOrientationVector(Up);
  }

  inline glm::vec3 Transform::GetRightVector() const
  {
    return GetOrientationVector(Right);
  }
};
