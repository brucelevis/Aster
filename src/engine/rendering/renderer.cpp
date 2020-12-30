#include "renderer.h"

#include <engine/components/camera_component.h>
#include <engine/components/static_mesh_component.h>
#include <engine/rendering/vulkan/fileutils.h>

#include <ecs/Context.h>

namespace
{
  struct PerStaticMeshResource
  {
    glm::mat4 mvp;
  };
}

RenderSystem::RenderSystem(Context* ctx, Core& vkCore)
  : LogicSystem(ctx)
  , vkCore(vkCore)
{
  cameraGroup = ctx->GetGroup<CameraComponent>();
  staticMeshGroup = ctx->GetGroup<StaticMeshComponent>();

  Shader vertexShader = vkCore.CreateShader("static_mesh_vertex", ReadFile("../data/shaders/spirv/static_mesh.vert.spv"));
  Shader fragmentShader = vkCore.CreateShader("static_mesh_fragment", ReadFile("../data/shaders/spirv/static_mesh.frag.spv"));
  staticMeshShaderProgram = std::make_unique<ShaderProgram>(vkCore, std::move(vertexShader), std::move(fragmentShader));
}

void RenderSystem::Update(const double dt)
{
  Entity* cameraEntity = cameraGroup->GetFirstNotNullEntity();
  CameraComponent* camera = cameraEntity->GetFirstComponent<CameraComponent>();

  if (camera == nullptr)
    throw std::runtime_error("Camera is not set.");

  RenderGraph* rg = vkCore.BeginFrame();
  rg->AddRenderSubpass()
    .AddOutputColorAttachment(OutputColorAttachmentDescription{ BACKBUFFER_RESOURCE_ID })
    .AddDepthStencilAttachment({"depth"})
    .SetRenderCallback([&](FrameContext& context)
     {
        VertexInputDeclaration vid = StaticMeshVertex::GetVID();

        Pipeline* pipeline = context.pipelineStorage->GetPipeline(*staticMeshShaderProgram, vid, vk::PrimitiveTopology::eTriangleList, EnableDepthTest, context);
        UniformsAccessor* uniforms = context.uniformsAccessorStorage->GetUniformsAccessor(*staticMeshShaderProgram);

        RenderStaticMeshes(camera, pipeline, uniforms, context.commandBuffer);
     });

  vkCore.EndFrame();
}


void RenderSystem::RenderStaticMeshes(CameraComponent* camera, Pipeline* pipeline, UniformsAccessor* uniforms, vk::CommandBuffer& commandBuffer)
{
  commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline->GetPipeline());

  for (Entity* e : staticMeshGroup->GetEntities())
  {
    if (e == nullptr)
      continue;

    for (auto* meshComponent : e->GetComponents<StaticMeshComponent>())
    {
      const glm::mat4 model = meshComponent->transform.GetTransformationMatrix();
      const glm::mat4 view = camera->GetView();
      const glm::mat4 projection = camera->GetProjection();

      PerStaticMeshResource mvpResource;
      mvpResource.mvp = projection * view * model;

      uniforms->SetUniformBuffer("PerStaticMeshResource", &mvpResource);
      uniforms->SetSampler2D("MeshTexture", meshComponent->mesh->texture);
      std::vector<vk::DescriptorSet> descriptorSets = uniforms->GetUpdatedDescriptorSets();

      commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline->GetLayout(), 0, descriptorSets.size(), descriptorSets.data(), 0, nullptr);
      vk::DeviceSize offset = 0;
      commandBuffer.bindVertexBuffers(0, 1, &meshComponent->mesh->vertices.GetBuffer(), &offset);
      commandBuffer.bindIndexBuffer(meshComponent->mesh->indices.GetBuffer(), 0, vk::IndexType::eUint32);
      commandBuffer.drawIndexed(meshComponent->mesh->indexCount, 1, 0, 0, 0);
    }
  }
}