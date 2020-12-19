#include "camera_component.h"

glm::mat4 CameraComponent::GetView() const
{
  glm::vec3 location = transform.GetWorldPosition();
  location = -location;

  glm::mat4 mat(1);
  mat = glm::translate(mat, location);

  glm::quat rotation = transform.GetWorldRotation();
  rotation = glm::inverse(rotation);
  glm::mat4 rotationMatrix = glm::mat4_cast(rotation);

  mat = rotationMatrix * mat;

  return mat;
}

glm::mat4 CameraComponent::GetProjection() const
{
  return glm::perspective(angle, width / height, zNear, zFar);
}