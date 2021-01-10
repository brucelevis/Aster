#include "camera_component.h"

#include <engine/math/math.h>

glm::mat4 CameraComponent::GetView() const
{
  return transform.GetCameraTransformationMatrixWithoutScale();
}

glm::mat4 CameraComponent::GetProjection() const
{
  return Math::Perspective(angle, width / height, zNear, zFar);
}