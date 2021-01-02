#pragma once

#include "common.h"

#include <vulkan/vulkan.hpp>

class RenderGraph;
class UniformsAccessorStorage;
class PipelineStorage;
class ImageView;

struct FrameContext
{
  friend class RenderGraph;

  const ImageView& GetImageView(const ResourceId& id) const;

  vk::Extent2D BackbufferSize;
  UniformsAccessorStorage* uniformsAccessorStorage;
  PipelineStorage* pipelineStorage;
  vk::RenderPass renderPass;
  vk::Framebuffer framebuffer;
  vk::CommandBuffer commandBuffer;
  uint32_t subpassNumber;

private:
  RenderGraph* renderGraph;
};