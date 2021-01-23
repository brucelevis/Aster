#pragma once

#include <engine/rendering/vulkan/core.h>

namespace RHI::Vulkan
{
  class Core;
}

namespace App
{
  class Scene;
  class Camera;
}

namespace Rendering
{
  class SceneRenderer
  {
  public:
    SceneRenderer(RHI::Vulkan::Core* vkCore);

    void Render(const App::Scene& scene, const App::Camera& camera);

  private:
    RHI::Vulkan::Core* m_VkCore;

    std::unique_ptr<RHI::Vulkan::ShaderProgram> m_SceneLinesProgram;

    struct
    {
      RHI::Vulkan::Buffer vertices;
      uint32_t vertexCount;
    } m_SceneLines;
  };
}