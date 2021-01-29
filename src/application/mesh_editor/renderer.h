#pragma once

#include <engine/rendering/vulkan/core.h>

namespace RHI::Vulkan
{
  class Core;
}

namespace Editor
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

    void Render(Editor::Scene& scene, const Editor::Camera& camera);

  private:
    void RenderEditorWires(RHI::Vulkan::FrameContext& ctx, const Editor::Camera& camera);
    void RenderSceneObjects(RHI::Vulkan::FrameContext& ctx, const Editor::Camera& camera, Editor::Scene& scene);

    void SyncObjects(Editor::Scene& scene);

  private:
    RHI::Vulkan::Core* m_VkCore;

    std::unique_ptr<RHI::Vulkan::ShaderProgram> m_SceneLinesProgram;
    std::unique_ptr<RHI::Vulkan::ShaderProgram> m_StaticMeshProgram;

    struct
    {
      RHI::Vulkan::Buffer vertices;
      uint32_t vertexCount;
    } m_SceneLines;
  };
}