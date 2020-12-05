#pragma once

#include <vulkan/vulkan.hpp>

class UniformsContextStorage;
class PipelineStorage;

struct FrameContext
{
  vk::Extent2D BackbufferSize;
  UniformsContextStorage* uniformsContextStorage;
  PipelineStorage* pipelineStorage;
  vk::RenderPass renderPass;
  vk::Framebuffer framebuffer;
  vk::CommandBuffer commandBuffer;
  uint32_t subpassNumber;
};