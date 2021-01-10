#pragma once

#include <engine/components/transform.h>

#include <ecs/BaseComponent.h>

struct CameraComponent : public BaseComponent
{
  Transform transform;
  float angle;
  float width, height;
  float zNear, zFar;

  glm::mat4 GetView() const;
  glm::mat4 GetProjection() const;
};