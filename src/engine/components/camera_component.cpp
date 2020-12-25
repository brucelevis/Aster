#include "camera_component.h"

glm::mat4 CameraComponent::GetView() const
{
  return transform.GetCameraTransformationMatrixWithoutScale();
}

glm::mat4 CameraComponent::GetProjection() const
{
  return glm::perspective(angle, width / height, zNear, zFar);
}