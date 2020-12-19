#include "transform.h"

#include "glm/gtc/matrix_transform.hpp"
#include <assert.h>

Transform::Transform()
  : Parent(nullptr)
  , LocalPosition(0.0f, 0.0f, 0.0f)
  , LocalScale(1.0f, 1.0f, 1.0f)

{
}

//TODO when parent component will be removed from the entity,
// one have to remove all links between parent,childs
void Transform::AttachTo(Transform* parent)
{
  Parent = parent;
}

glm::vec3 Transform::GetWorldPosition() const
{
  glm::vec3 worldPosition = LocalPosition;
  Transform* parent = Parent;
  while (parent != nullptr)
  {
    worldPosition += parent->LocalPosition;
    parent = parent->Parent;
  }

  return worldPosition;
}

glm::quat Transform::GetWorldRotation() const
{
  glm::quat worldRotation = LocalRotation;
  Transform* parent = Parent;
  while (parent != nullptr)
  {
    worldRotation *= parent->LocalRotation;
    parent = parent->Parent;
  }

  return worldRotation;
}

glm::vec3 Transform::GetWorldScale() const
{
  glm::vec3 worldScale = LocalScale;
  Transform* parent = Parent;
  while (parent != nullptr)
  {
    worldScale *= parent->LocalScale;
    parent = parent->Parent;
  }

  return worldScale;
}

glm::mat4 Transform::GetTransformationMatrix() const
{
  const glm::vec3 worldScale = GetWorldScale();
  glm::mat4 mat = GetTransformationMatrixWithoutScale();
  mat = glm::scale(mat, worldScale);

  return mat;
}

glm::mat4 Transform::GetTransformationMatrixWithoutScale() const
{
  const glm::vec3 worldPosition = GetWorldPosition();
  const glm::quat worldRotation = GetWorldRotation();

  glm::mat4 mat(1.0f);
  mat = glm::translate(mat, worldPosition);
  mat = glm::mat4_cast(worldRotation) * mat;

  return mat;
}

//Vulkan orientation
glm::vec3 Transform::GetOrientationVector(OrientationVectorType type) const
{
  const glm::mat4 mat = GetTransformationMatrixWithoutScale();
  switch (type)
  {
  case Forward:
    return glm::vec3{ mat * glm::vec4{ 0.0f, 0.0f, 1.0f, 0.0f } };

  case Right:
    return glm::vec3{ mat * glm::vec4{ 1.0f, 0.0f, 0.0f, 0.0f } };

  case Up:
    return glm::vec3{ mat * glm::vec4{ 0.0f, -1.0f, 0.0f, 0.0f } };

  default:
    assert(!"Unknown orientation type");
  }
}