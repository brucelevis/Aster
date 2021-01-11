#pragma once

#include "Shader.h"
#include "common.h"

#include <vulkan/vulkan.hpp>

class VertexInputDeclaration;
class DepthStencilSettings;

class Pipeline
{
public:
  Pipeline(vk::Device logicalDevice,
    const ShaderProgram& program,
    const VertexInputDeclaration& vertexInputDeclaration,
    const std::vector<vk::DescriptorSetLayout> layouts,
    const vk::PrimitiveTopology topology,
    const DepthStencilSettings& depthStencilSettings,
    const vk::Extent2D viewportExtent,
    const vk::RenderPass renderpass,
    const uint32_t subpass,
    const uint32_t attachmentsCount);

  inline vk::Pipeline GetPipeline() const 
  { 
    return pipeline.get(); 
  }

  inline const PipelineUniforms& GetUniformsDescription() const
  {
    return uniformsDescriptions;
  }

  inline vk::PipelineLayout GetLayout() const
  {
    return layout.get();
  }

private:
  vk::UniquePipeline pipeline;
  vk::UniquePipelineLayout layout;
  PipelineUniforms uniformsDescriptions;
};