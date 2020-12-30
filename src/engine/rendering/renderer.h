#pragma once

#include <engine/rendering/vulkan/core.h>

#include <ecs/BaseSystems.h>

#include <glm/glm.hpp>
#include <memory>

class Context;
class Group;
class Core;
struct GLFWwindow;
struct CameraComponent;

class RenderSystem : public LogicSystem
{
public:
  RenderSystem(Context* ctx, Core& vkCore);

  virtual void Update(const double dt) override;

private:
  void RenderStaticMeshes(CameraComponent* camera, Pipeline* pipeline, UniformsAccessor* uniforms, vk::CommandBuffer& commandBuffer);

private:
  Core& vkCore;

  Group* cameraGroup;
  Group* staticMeshGroup;

  std::unique_ptr<ShaderProgram> staticMeshShaderProgram;
};