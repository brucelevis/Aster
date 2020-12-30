#include "camera.h"

#include <engine/engine.h>
#include <engine/components/camera_component.h>

#include <ecs/Context.h>

CameraMovementSystem::CameraMovementSystem(Context* pContext)
  : LogicSystem(pContext)
{
  cameraGroup = pContext->GetGroup<CameraComponent>();

  InputHandler* inputHandler = pContext->GetUserData<Engine*>()->GetInputHandler();
  inputHandler->DisableCursor();

  const float velocity = 5.0f;

  inputHandler->SetKeyBinding("W", velocity, [&](float value)
    {
      MoveForward(value);
    });

  inputHandler->SetKeyBinding("S", -velocity, [&](float value)
    {
      MoveForward(value);
    });

  inputHandler->SetKeyBinding("D", velocity, [&](float value)
    {
      MoveRight(value);
    });

  inputHandler->SetKeyBinding("A", -velocity, [&](float value)
    {
      MoveRight(value);
    });

  inputHandler->SetKeyBinding("SPACE", velocity, [&](float value)
    {
      MoveUp(value);
    });

  inputHandler->SetKeyBinding("C", -velocity, [&](float value)
    {
      MoveUp(value);
    });

  inputHandler->SetMouseInputBinding([&](float dx, float dy)
    {
      Rotate(dx, dy);
    });

  inputHandler->SetKeyBinding("ESCAPE", 0.0f, [](float) {
    exit(0);
    });
}

void CameraMovementSystem::MoveForward(float value)
{
  movingForwardValue = value;
}

void CameraMovementSystem::MoveRight(float value)
{
  movingRightValue = value;
}

void CameraMovementSystem::MoveUp(float value)
{
  movingUpValue = value;
}

void CameraMovementSystem::Rotate(float dx, float dy)
{
  CameraComponent* camera = cameraGroup->GetFirstNotNullEntity()->GetFirstComponent<CameraComponent>();

  if (dx > 0 || dx < 0)
  {
    angleX += dx * 0.1;

    if (angleX > 360.0f || angleX < -360.0f)
      angleX = 0.0f;
  }

  if (dy > 0 || dy < 0)
  {
    angleY += dy * 0.1;

    if (angleY > 90.0f)
      angleY = 90.0f;
    else if (angleY < -90.0f)
      angleY = -90.0f;
  }

  camera->transform.LocalRotation = glm::angleAxis(glm::radians(angleX), glm::vec3{ 0.0f, -1.0f, 0.0f })
    * glm::angleAxis(glm::radians(angleY), glm::vec3{ 1.0f, 0.0f, 0.0f });
}

void CameraMovementSystem::Update(const double dt)
{
  pContext
    ->GetUserData<Engine*>()
    ->GetInputHandler()
    ->PollEvents();

  CameraComponent* camera = cameraGroup->GetFirstNotNullEntity()->GetFirstComponent<CameraComponent>();

  if (movingForwardValue > 0 || movingForwardValue < 0)
    camera->transform.LocalPosition += (movingForwardValue * (float)dt) * camera->transform.GetForwardVector();

  if (movingRightValue > 0 || movingRightValue < 0)
    camera->transform.LocalPosition += (movingRightValue * (float)dt) * camera->transform.GetRightVector();

  if (movingUpValue > 0 || movingUpValue < 0)
    camera->transform.LocalPosition += (movingUpValue * (float)dt) * glm::vec3{ 0.0f, -1.0f, 0.0f };
}
