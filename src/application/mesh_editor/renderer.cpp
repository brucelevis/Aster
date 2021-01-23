#include "renderer.h"
#include "camera.h"

#include <engine/utils/mesh_generation.h>
#include <engine/rendering/vulkan/fileutils.h>

namespace
{
  RHI::Vulkan::VertexInputDeclaration GetSceneLinesVID()
  {
    RHI::Vulkan::VertexInputDeclaration vid;

    vid.AddBindingDescription(0, sizeof(glm::vec3));
    vid.AddAttributeDescription(vk::Format::eR32G32B32Sfloat, 0, 0, 0);

    return vid;
  }
}

namespace Rendering
{

  SceneRenderer::SceneRenderer(RHI::Vulkan::Core* vkCore)
    : m_VkCore(vkCore)
  {
    std::vector<glm::vec3> vertices = Utils::GenerateWirePlane(20, 20);
    m_SceneLines.vertices = m_VkCore->AllocateDeviceBuffer(vertices.data(), vertices.size() * sizeof(glm::vec3), vk::BufferUsageFlagBits::eVertexBuffer);
    m_SceneLines.vertexCount = vertices.size();

    auto sceneLinesVert = m_VkCore->CreateShader(RHI::Vulkan::ReadFile("../data/shaders/spirv/mesh_editor/sceneLines.vert.spv"));
    auto sceneLinesFrag = m_VkCore->CreateShader(RHI::Vulkan::ReadFile("../data/shaders/spirv/mesh_editor/sceneLines.frag.spv"));
    m_SceneLinesProgram = std::make_unique<RHI::Vulkan::ShaderProgram>(*m_VkCore, std::move(sceneLinesVert), std::move(sceneLinesFrag));
  }

  void SceneRenderer::Render(const App::Scene& scene, const App::Camera& camera)
  {
    RHI::Vulkan::RenderGraph* rg = m_VkCore->BeginFrame();

    rg->AddRenderSubpass()
      .AddExistOutputColorAttachment(BACKBUFFER_RESOURCE_ID)
      .SetRenderCallback([&](RHI::Vulkan::FrameContext& ctx) 
    {
      RHI::Vulkan::Pipeline* pipeline = ctx.GetPipeline(*m_SceneLinesProgram, GetSceneLinesVID(), vk::PrimitiveTopology::eLineList, RHI::Vulkan::DisableDepthTest);
      ctx.commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline->GetPipeline());

      struct
      {
        glm::mat4 Projection;
        glm::mat4 View;
      } cameraResource{
          camera.GetProjection(),
          camera.GetView()
      };

      auto c1 = cameraResource.Projection * glm::vec4(-16.0f, 5.0f, 20.0f, 1.0f);
      auto c2 = c1 / c1.w;

      RHI::Vulkan::UniformsAccessor* uniforms = ctx.GetUniformsAccessor(*m_SceneLinesProgram);
      uniforms->SetUniformBuffer("Camera", &cameraResource);
      std::vector<vk::DescriptorSet> descriptorSets = uniforms->GetUpdatedDescriptorSets();

      ctx.commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline->GetLayout(), 0, descriptorSets.size(), descriptorSets.data(), 0, nullptr);

      vk::DeviceSize offset = 0;
      ctx.commandBuffer.bindVertexBuffers(0, 1, &m_SceneLines.vertices.GetBuffer(), &offset);

      ctx.commandBuffer.draw(m_SceneLines.vertexCount, 1, 0, 0);
    });
  
    m_VkCore->EndFrame();
  }
}