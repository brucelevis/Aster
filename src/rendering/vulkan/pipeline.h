#pragma once

#include "Shader.h"

#include <vulkan/vulkan.hpp>

class VertexInputDeclaration;

struct DepthStencilSettings
{
  bool depthTestEnabled = false;
  bool depthWriteEnabled = false;

  inline DepthStencilSettings& SetDepthTestEnabled(bool b) {
    depthTestEnabled = b;
    return *this;
  }

  inline DepthStencilSettings& SetDepthWriteEnabled(bool b) {
    depthWriteEnabled = b;
    return *this;
  }
};

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
    const uint32_t subpass);

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