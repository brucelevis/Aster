#pragma once

#include <engine/rendering/vulkan/core.h>

#include <ecs/BaseSystems.h>

#include <glm/glm.hpp>
#include <memory>

class Context;
class Core;
struct GLFWwindow;

class RenderSystem : public LogicSystem
{
public:
  RenderSystem(Context* ctx, Core& vkCore);

  virtual void Update(const double dt) override;

private:
  Core& vkCore;

  std::unique_ptr<ShaderProgram> static_mesh_shader_program;
};