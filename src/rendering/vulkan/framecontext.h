#pragma once

#include <vulkan/vulkan.hpp>

class UniformsAccessorStorage;
class PipelineStorage;

struct FrameContext
{
  vk::Extent2D BackbufferSize;
  UniformsAccessorStorage* uniformsAccessorStorage;
  PipelineStorage* pipelineStorage;
  vk::RenderPass renderPass;
  vk::Framebuffer framebuffer;
  vk::CommandBuffer commandBuffer;
  uint32_t subpassNumber;
};