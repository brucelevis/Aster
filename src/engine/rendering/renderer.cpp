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

  {
    Shader vertexShader = vkCore.CreateShader("static_mesh_gbuffer_vertex", ReadFile("../data/shaders/spirv/static_mesh_gbuffer.vert.spv"));
    Shader fragmentShader = vkCore.CreateShader("static_mesh_gbuffer_fragment", ReadFile("../data/shaders/spirv/static_mesh_gbuffer.frag.spv"));
    staticMeshShaderGbufferProgram = std::make_unique<ShaderProgram>(vkCore, std::move(vertexShader), std::move(fragmentShader));
  }

  {
    Shader vertexShader = vkCore.CreateShader("static_mesh_vertex", ReadFile("../data/shaders/spirv/static_mesh.vert.spv"));
    Shader fragmentShader = vkCore.CreateShader("static_mesh_fragment", ReadFile("../data/shaders/spirv/static_mesh.frag.spv"));
    staticMeshShaderProgram = std::make_unique<ShaderProgram>(vkCore, std::move(vertexShader), std::move(fragmentShader));
  }
}

void RenderSystem::Update(const double dt)
{
  Entity* cameraEntity = cameraGroup->GetFirstNotNullEntity();
  CameraComponent* camera = cameraEntity->GetFirstComponent<CameraComponent>();

  if (camera == nullptr)
    throw std::runtime_error("Camera is not set.");

  RenderGraph* rg = vkCore.BeginFrame();
  /*rg->AddRenderSubpass()
    .AddOutputColorAttachment(OutputColorAttachmentDescription{ BACKBUFFER_RESOURCE_ID })
    .AddDepthStencilAttachment({"depth"})
    .SetRenderCallback([&](FrameContext& context)
     {
        VertexInputDeclaration vid = StaticMeshVertex::GetVID();

        Pipeline* pipeline = context.pipelineStorage->GetPipeline(*staticMeshShaderProgram, vid, vk::PrimitiveTopology::eTriangleList, EnableDepthTest, context);
        UniformsAccessor* uniforms = context.uniformsAccessorStorage->GetUniformsAccessor(*staticMeshShaderProgram);

        RenderStaticMeshes(camera, pipeline, uniforms, context.commandBuffer);
     });
     */

  rg->AddRenderSubpass()
    .AddOutputColorAttachment(ImageAttachmentDescription{"GBUFFER_BaseColor" })
    .AddOutputColorAttachment(ImageAttachmentDescription{"GBUFFER_Normal" })
    .AddOutputColorAttachment(ImageAttachmentDescription{"GBUFFER_Metallic"})
    .AddOutputColorAttachment(ImageAttachmentDescription{"GBUFFER_Roughness"})
    .AddOutputColorAttachment(ImageAttachmentDescription{"GBUFFER_Depth"})
    .AddDepthStencilAttachment({ "depth" })
    .SetRenderCallback([&](FrameContext& context)
     {
       VertexInputDeclaration vid = StaticMeshVertex::GetVID();

       Pipeline* pipeline = context.pipelineStorage->GetPipeline(*staticMeshShaderGbufferProgram, vid, vk::PrimitiveTopology::eTriangleList, EnableDepthTest, 5, context);
       UniformsAccessor* uniforms = context.uniformsAccessorStorage->GetUniformsAccessor(*staticMeshShaderProgram);

       RenderStaticMeshes(camera, pipeline, uniforms, context.commandBuffer);
     });

  rg->AddRenderSubpass()
    .AddInputAttachment(InputAttachmentDescription{ "GBUFFER_BaseColor", vk::ImageLayout::eShaderReadOnlyOptimal })
    .AddInputAttachment(InputAttachmentDescription{ "GBUFFER_Normal", vk::ImageLayout::eShaderReadOnlyOptimal })
    .AddInputAttachment(InputAttachmentDescription{ "GBUFFER_Metallic", vk::ImageLayout::eShaderReadOnlyOptimal })
    .AddInputAttachment(InputAttachmentDescription{ "GBUFFER_Roughness", vk::ImageLayout::eShaderReadOnlyOptimal })
    .AddInputAttachment(InputAttachmentDescription{ "GBUFFER_Depth", vk::ImageLayout::eShaderReadOnlyOptimal })
    .AddOutputColorAttachment(OutputColorAttachmentDescription{ BACKBUFFER_RESOURCE_ID })
    .AddDepthStencilAttachment({ "depth2" })
    .SetRenderCallback([&](FrameContext& context)
      {
        VertexInputDeclaration vid = StaticMeshVertex::GetVID();

        Pipeline* pipeline = context.pipelineStorage->GetPipeline(*staticMeshShaderProgram, vid, vk::PrimitiveTopology::eTriangleList, EnableDepthTest, 1, context);
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

      for (int i = 0; i < meshComponent->model->meshes.size(); ++i)
      {
        const StaticMesh& mesh = meshComponent->model->meshes[i];
        const Material& meshMaterial = meshComponent->model->materials[i];

        assert(meshMaterial.colorTexture != nullptr);

        uniforms->SetSampler2D("BaseColorTexture", *meshMaterial.colorTexture);
        uniforms->SetSampler2D("NormalTexture", *meshMaterial.normalTexture);
        uniforms->SetSampler2D("MetallicRoughnessTexture", *meshMaterial.metallicRoughnessTexture);
        std::vector<vk::DescriptorSet> descriptorSets = uniforms->GetUpdatedDescriptorSets();

        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline->GetLayout(), 0, descriptorSets.size(), descriptorSets.data(), 0, nullptr);
        vk::DeviceSize offset = 0;
        commandBuffer.bindVertexBuffers(0, 1, &mesh.vertices.GetBuffer(), &offset);
        commandBuffer.bindIndexBuffer(mesh.indices.GetBuffer(), 0, vk::IndexType::eUint32);
        commandBuffer.drawIndexed(mesh.indexCount, 1, 0, 0, 0);
      }
    }
  }
}