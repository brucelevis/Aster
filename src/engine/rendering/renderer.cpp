#include "renderer.h"

#include <engine/components/camera_component.h>
#include <engine/components/static_mesh_component.h>
#include <engine/components/sky_box_component.h>
#include <engine/rendering/vulkan/fileutils.h>

#include <ecs/Context.h>

namespace
{
  struct PerStaticMeshResource
  {
    glm::mat4 projection;
    glm::mat4 view;
    glm::mat4 model;
  };

  struct SkyboxPerFrameResource
  {
    glm::mat4 projection;
    glm::mat4 view;
    glm::mat4 model;
  };
}

RenderSystem::RenderSystem(Context* ctx, RHI::Vulkan::Core& vkCore)
  : LogicSystem(ctx)
  , vkCore(vkCore)
{
  cameraGroup = ctx->GetGroup<CameraComponent>();
  staticMeshGroup = ctx->GetGroup<RHI::Vulkan::StaticMeshComponent>();
  skyboxGroup = ctx->GetGroup<RHI::Vulkan::SkyBoxComponent>();

  {
    RHI::Vulkan::Shader vertexShader = vkCore.CreateShader(RHI::Vulkan::ReadFile("../data/shaders/spirv/static_mesh_gbuffer.vert.spv"));
    RHI::Vulkan::Shader fragmentShader = vkCore.CreateShader(RHI::Vulkan::ReadFile("../data/shaders/spirv/static_mesh_gbuffer.frag.spv"));
    staticMeshShaderGbufferProgram = std::make_unique<RHI::Vulkan::ShaderProgram>(vkCore, std::move(vertexShader), std::move(fragmentShader));
  }

  {
    RHI::Vulkan::Shader vertexShader = vkCore.CreateShader(RHI::Vulkan::ReadFile("../data/shaders/spirv/deferred_light.vert.spv"));
    RHI::Vulkan::Shader fragmentShader = vkCore.CreateShader(RHI::Vulkan::ReadFile("../data/shaders/spirv/deferred_light.frag.spv"));
    deferredLightProgram = std::make_unique<RHI::Vulkan::ShaderProgram>(vkCore, std::move(vertexShader), std::move(fragmentShader));
  }

  {
    RHI::Vulkan::Shader vertexShader = vkCore.CreateShader(RHI::Vulkan::ReadFile("../data/shaders/spirv/sky_box.vert.spv"));
    RHI::Vulkan::Shader fragmentShader = vkCore.CreateShader(RHI::Vulkan::ReadFile("../data/shaders/spirv/sky_box.frag.spv"));
    skyBoxShaderProgram = std::make_unique<RHI::Vulkan::ShaderProgram>(vkCore, std::move(vertexShader), std::move(fragmentShader));
  }
}

void RenderSystem::Update(const double dt)
{
  Entity* cameraEntity = cameraGroup->GetFirstNotNullEntity();
  CameraComponent* camera = cameraEntity->GetFirstComponent<CameraComponent>();

  if (camera == nullptr)
    throw std::runtime_error("Camera is not set.");

  RHI::Vulkan::RenderGraph* rg = vkCore.BeginFrame();
  RenderGBuffer(camera, rg);
  RenderLight(camera, rg);

  vkCore.EndFrame();
}


void RenderSystem::RenderGBuffer(CameraComponent* camera, RHI::Vulkan::RenderGraph* rg)
{
  rg->AddRenderSubpass()
    .AddNewOutputColorAttachment("GBUFFER_BaseColor")
    .AddNewOutputColorAttachment("GBUFFER_WorldPosition")
    .AddNewOutputColorAttachment("GBUFFER_WorldNormal")
    .AddNewOutputColorAttachment("GBUFFER_Metallic")
    .AddNewOutputColorAttachment("GBUFFER_Roughness")
    .AddNewOutputColorAttachment("GBUFFER_Depth")
    .AddDepthStencilAttachment("depth")
    .SetRenderCallback([&](RHI::Vulkan::FrameContext& context)
    {
      vk::CommandBuffer& commandBuffer = context.commandBuffer;
      RHI::Vulkan::VertexInputDeclaration vid = RHI::Vulkan::StaticMeshVertex::GetVID();

      RHI::Vulkan::Pipeline* pipeline = context.GetPipeline(*staticMeshShaderGbufferProgram, vid, vk::PrimitiveTopology::eTriangleList, RHI::Vulkan::EnableDepthTest, RHI::Vulkan::FillMode);
      RHI::Vulkan::UniformsAccessor* uniforms = context.GetUniformsAccessor(*staticMeshShaderGbufferProgram);

      commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline->GetPipeline());

      for (Entity* e : staticMeshGroup->GetEntities())
      {
        if (e == nullptr)
          continue;

        for (auto* meshComponent : e->GetComponents<RHI::Vulkan::StaticMeshComponent>())
        {
          const glm::mat4 model = meshComponent->transform.GetTransformationMatrix();
          const glm::mat4 view = camera->GetView();
          const glm::mat4 projection = camera->GetProjection();

          PerStaticMeshResource mvpResource;
          mvpResource.projection = projection;
          mvpResource.view = view;
          mvpResource.model = model;

          uniforms->SetUniformBuffer("PerStaticMeshResource", &mvpResource);

          for (int i = 0; i < meshComponent->model->meshes.size(); ++i)
          {
            const RHI::Vulkan::StaticMesh& mesh = meshComponent->model->meshes[i];
            const RHI::Vulkan::Material& meshMaterial = meshComponent->model->materials[i];

            assert(meshMaterial.colorTexture != nullptr);

            uniforms->SetSampler2D("BaseColorTexture", *meshMaterial.colorTexture);
            uniforms->SetSampler2D("NormalTexture", *meshMaterial.normalTexture);
            uniforms->SetSampler2D("MetallicRoughnessTexture", *meshMaterial.metallicRoughnessTexture);
            std::vector<vk::DescriptorSet> descriptorSets = uniforms->GetUpdatedDescriptorSets();

            commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline->GetLayout(), 0, descriptorSets.size(), descriptorSets.data(), 0, nullptr);
            vk::DeviceSize offset = 0;
            commandBuffer.bindVertexBuffers(0, 1, &mesh.vertices.GetBuffer(), &offset);
            commandBuffer.bindVertexBuffers(1, 1, &mesh.tbnVectorsBuffer.GetBuffer(), &offset);
            commandBuffer.bindIndexBuffer(mesh.indices.GetBuffer(), 0, vk::IndexType::eUint32);
            commandBuffer.drawIndexed(mesh.indexCount, 1, 0, 0, 0);
          }
        }
      }

      //render skybox
      {
        RHI::Vulkan::SkyBoxComponent* skybox = skyboxGroup->GetFirstNotNullEntity()->GetFirstComponent<RHI::Vulkan::SkyBoxComponent>();
        RHI::Vulkan::VertexInputDeclaration vid = RHI::Vulkan::SkyBoxVertex::GetVID();

        RHI::Vulkan::Pipeline* pipeline = context.GetPipeline(*skyBoxShaderProgram, vid, vk::PrimitiveTopology::eTriangleList, RHI::Vulkan::EnableDepthTest, RHI::Vulkan::FillMode);
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline->GetPipeline());

        RHI::Vulkan::UniformsAccessor* uniforms = context.GetUniformsAccessor(*skyBoxShaderProgram);

        SkyboxPerFrameResource perFrameUbo;
        perFrameUbo.projection = camera->GetProjection();
        perFrameUbo.view = camera->GetView();
        perFrameUbo.model = skybox->transform.GetTransformationMatrix();

        uniforms->SetUniformBuffer("PerFrame", &perFrameUbo);
        uniforms->SetSamplerCube("SkyboxTexture", skybox->cubeMap->GetView());

        std::vector<vk::DescriptorSet> descriptorSets = uniforms->GetUpdatedDescriptorSets();
        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline->GetLayout(), 0, descriptorSets.size(), descriptorSets.data(), 0, nullptr);

        vk::DeviceSize offset = 0;
        commandBuffer.bindVertexBuffers(0, 1, &skybox->skyboxMesh->vertices.GetBuffer(), &offset);
        commandBuffer.bindIndexBuffer(skybox->skyboxMesh->indices.GetBuffer(), 0, vk::IndexType::eUint32);
        commandBuffer.drawIndexed(skybox->skyboxMesh->indexCount, 1, 0, 0, 0);
      }
    });
}

void RenderSystem::RenderLight(CameraComponent* camera, RHI::Vulkan::RenderGraph* rg)
{
  rg->AddRenderSubpass()
    .AddInputAttachment({"GBUFFER_BaseColor"})
    .AddInputAttachment({"GBUFFER_WorldPosition"})
    .AddInputAttachment({"GBUFFER_WorldNormal"})
    .AddInputAttachment({"GBUFFER_Metallic"})
    .AddInputAttachment({"GBUFFER_Roughness"})
    .AddInputAttachment({"GBUFFER_Depth"})
    .AddExistOutputColorAttachment(BACKBUFFER_RESOURCE_ID)
    .SetRenderCallback([&](RHI::Vulkan::FrameContext& context)
    {
      vk::CommandBuffer& commandBuffer = context.commandBuffer;

      RHI::Vulkan::Pipeline* pipeline = context.GetPipeline(*deferredLightProgram, RHI::Vulkan::VertexInputDeclaration{}, vk::PrimitiveTopology::eTriangleStrip, RHI::Vulkan::DisableDepthTest, RHI::Vulkan::FillMode);
      RHI::Vulkan::UniformsAccessor* uniforms = context.GetUniformsAccessor(*deferredLightProgram);

      commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline->GetPipeline());

      uniforms->SetSubpassInput("BaseColorTexture", context.GetImageView("GBUFFER_BaseColor"));
      uniforms->SetSubpassInput("WorldPositionTexture", context.GetImageView("GBUFFER_WorldPosition"));
      uniforms->SetSubpassInput("WorldNormalTexture", context.GetImageView("GBUFFER_WorldNormal"));
      uniforms->SetSubpassInput("MetallicTexture", context.GetImageView("GBUFFER_Metallic"));
      uniforms->SetSubpassInput("RoughnessTexture", context.GetImageView("GBUFFER_Roughness"));

      std::vector<vk::DescriptorSet> descriptorSets = uniforms->GetUpdatedDescriptorSets();
      commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline->GetLayout(), 0, descriptorSets.size(), descriptorSets.data(), 0, nullptr);

      vk::DeviceSize offset = 0;
      commandBuffer.draw(4, 1, 0, 0);
    });
}
