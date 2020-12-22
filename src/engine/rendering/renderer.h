#pragma once

#include <engine/rendering/vulkan/core.h>

#include <ecs/BaseSystems.h>

#include <glm/glm.hpp>
#include <memory>

class Context;
class Core;
struct GLFWwindow;

struct StaticMeshVertex
{
  alignas(16) glm::vec3 position;
  alignas(16) glm::vec3 color;

  static inline VertexInputDeclaration GetVID()
  {
    VertexInputDeclaration vid;
    vid.AddBindingDescription(0, sizeof(StaticMeshVertex));
    vid.AddAttributeDescription(vk::Format::eR32G32B32Sfloat, 0, 0, offsetof(StaticMeshVertex, position));
    vid.AddAttributeDescription(vk::Format::eR32G32B32Sfloat, 0, 1, offsetof(StaticMeshVertex, color));

    return vid;
  }
};

class RenderSystem : public LogicSystem
{
public:
  RenderSystem(Context* ctx, Core& vkCore);

  virtual void Update(const double dt) override;

private:
  Core& vkCore;

  std::unique_ptr<ShaderProgram> static_mesh_shader_program;
};