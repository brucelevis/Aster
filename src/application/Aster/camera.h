#pragma once

#include <ecs/BaseSystems.h>

class Context;
class Group;

class CameraMovementSystem : public LogicSystem
{
public:
  CameraMovementSystem(Context* pContext);

  void MoveForward(float value);

  void MoveRight(float value);

  void MoveUp(float value);

  void Rotate(float dx, float dy);

  virtual void Update(const double dt) override;

private:
  Group* cameraGroup;
  float movingForwardValue = 0.0f;
  float movingRightValue = 0.0f;
  float movingUpValue = 0.0f;

  float angleX = 0.0f;
  float angleY = 0.0f;
};