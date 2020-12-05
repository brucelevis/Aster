#pragma once

#include "Shader.h"
#include "pipeline.h"

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

#include <tuple>
#include <map>

class Core;
class DescriptorSetStorage;
struct FrameContext;

class PipelineKey
{
  friend class PipelineStorage;
public:
  PipelineKey& SetVertexShaderName(const std::string& s);

  PipelineKey& SetFragmentShaderName(const std::string s);

  PipelineKey& SetVertexInputDeclaration(const VertexInputDeclaration& d);

  PipelineKey& SetTopology(const vk::PrimitiveTopology t);

  PipelineKey& SetViewportExtent(const vk::Extent2D e);

  PipelineKey& SetRenderPass(const vk::RenderPass r);

  PipelineKey& SetSubpassNumber(const uint32_t n);

  bool operator<(const PipelineKey& r) const;

private:
  std::string vertexShader;
  std::string fragmentShader;
  VertexInputDeclaration vertexInputDeclaration;
  vk::PrimitiveTopology topology;
  vk::Extent2D viewportExtent;
  vk::RenderPass renderpass;
  uint32_t subpass;
};

class PipelineStorage
{
public:
  PipelineStorage(Core& core, DescriptorSetStorage& dsStorage);

  Pipeline* GetPipeline(const ShaderProgram& program, const VertexInputDeclaration& vertexInputDeclaration, vk::PrimitiveTopology topology, const vk::Extent2D& viewportExtent, vk::RenderPass renderPass, uint32_t subpassNumber);
  Pipeline* GetPipeline(const ShaderProgram& program, const VertexInputDeclaration& vertexInputDeclaration, vk::PrimitiveTopology topology, const FrameContext& frameContext);

private:
  Core& core;
  DescriptorSetStorage& dsStorage;

  std::map<PipelineKey, std::unique_ptr<Pipeline>> storage;
};