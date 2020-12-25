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

  glm::vec3 LocalPosition;
  glm::quat LocalRotation;
  glm::vec3 LocalScale;

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
