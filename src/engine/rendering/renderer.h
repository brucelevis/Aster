#pragma once

#include <engine/rendering/vulkan/core.h>

#include <ecs/BaseSystems.h>

#include <memory>

namespace RHI::Vulkan
{
  class Core;
}

class Context;
class Group;
struct GLFWwindow;
struct CameraComponent;

class RenderSystem : public LogicSystem
{
public:
  RenderSystem(Context* ctx, RHI::Vulkan::Core& vkCore);

  virtual void Update(const double dt) override;

private:
  void RenderStaticMeshes(CameraComponent* camera, RHI::Vulkan::Pipeline* pipeline, RHI::Vulkan::UniformsAccessor* uniforms, vk::CommandBuffer& commandBuffer);

  void RenderGBuffer(CameraComponent* camera, RHI::Vulkan::RenderGraph* rg);
  void RenderLight(CameraComponent* camera, RHI::Vulkan::RenderGraph* rg);

private:
  RHI::Vulkan::Core& vkCore;

  Group* cameraGroup;
  Group* staticMeshGroup;
  Group* skyboxGroup;

  std::unique_ptr<RHI::Vulkan::ShaderProgram> staticMeshShaderGbufferProgram;
  std::unique_ptr<RHI::Vulkan::ShaderProgram> skyBoxShaderProgram;
  std::unique_ptr<RHI::Vulkan::ShaderProgram> deferredLightProgram;
};