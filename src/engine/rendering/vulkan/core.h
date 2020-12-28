#pragma once

#include "framegraph.h"
#include "framebuffer_storage.h"
#include "pipeline_storage.h"
#include "renderpass_storage.h"
#include "uniforms_accessor_storage.h"

#include "swapchain.h"
#include "Shader.h"
#include "buffer.h"

#include <memory>
#include <stdint.h>
#include <set>

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>
#undef VK_USE_PLATFORM_WIN32_KHR

struct FrameResources
{
  vk::ImageView swapchainImage;
  vk::UniqueFence cmdBufferFreeToUse;
  vk::UniqueSemaphore swapchainImageAckquired;
  vk::UniqueSemaphore renderingFinished;
  vk::UniqueCommandBuffer cmdBuffer;
  std::unique_ptr<UniformsAccessorStorage> uaStorage;
  std::unique_ptr<RenderGraph> renderGraph;

};

struct GLFWwindow;

class Core
{
public:

  Core(GLFWwindow* window, const char** instanceExtensions, uint32_t instanceExtensionsCount, vk::Extent2D windowSize);

  FramebufferStorage& GetFramebufferStorage();

  RenderPassStorage& GetRenderPassStorage();

  PipelineStorage& GetPipelineStorage();

  vk::Device GetDebugDevice();

  vk::Format GetDebugSurfaceFormat();

  Shader CreateShader(const std::string& name, const std::vector<uint32_t>& byteCode);

  RenderGraph* BeginFrame();

  void EndFrame();

  HostBuffer AllocateHostBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage);

  Buffer AllocateDeviceBuffer(const void* src, vk::DeviceSize size, vk::BufferUsageFlags usage);

  Image AllocateImage(vk::ImageType type, vk::Format format, const vk::Extent3D& extent, vk::ImageUsageFlags usage, vk::ImageAspectFlags aspectMask);

  Image Allocate2DImage(vk::Format format, vk::Extent2D extent, vk::ImageUsageFlags usage);

  Image Allocate2DImage(void* src, vk::DeviceSize size, vk::Format format, vk::Extent2D extent, vk::ImageUsageFlags usage);

  Image AllocateDepthStencilImage(vk::Format format, vk::Extent2D extent);

  vk::PhysicalDevice GetPhysicalDevice(vk::Instance instance);

  vk::Device GetLogicalDevice() const;

private:
  std::tuple<vk::UniqueBuffer, vk::UniqueDeviceMemory, vk::DeviceSize> AllocateBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, uint32_t queueFamilyIndex, uint32_t memoryTypeIndex);

private:
  vk::UniqueInstance instance;
  vk::PhysicalDevice physicalDevice;
  vk::UniqueSurfaceKHR surface;
  uint32_t graphicsFamilyIndex;
  uint32_t presentFamilyIndex;
  uint32_t transferFamilyIndex;
  vk::UniqueDevice logicalDevice;
  vk::Queue graphicsQueue;
  vk::Queue presentQueue;
  vk::Queue transferQueue;
  vk::UniqueCommandPool cmdPool;
  std::unique_ptr<Swapchain> swapchain;
  vk::UniqueDescriptorPool descriptorPool;

  uint32_t currentVirtualFrame;
  std::vector<FrameResources> frameResources;

  std::unique_ptr<FramebufferStorage> fbStorage;
  std::unique_ptr<RenderPassStorage> rpStorage;
  std::unique_ptr<PipelineStorage> ppStorage;

  uint32_t hostVisibleMemoryIndex;
  uint32_t deviceLocalMemoryIndex;
};