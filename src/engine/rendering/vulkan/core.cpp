#include "core.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

Core::Core(GLFWwindow* window, const char** instanceExtensions, uint32_t instanceExtensionsCount, vk::Extent2D windowSize)
{
  HINSTANCE hInstance = GetModuleHandle(NULL);
  HWND hWnd = glfwGetWin32Window(window);

  std::vector<const char*> validationLayers{ "VK_LAYER_KHRONOS_validation" };

  //physical device
  {
    const auto appInfo = vk::ApplicationInfo()
      .setPApplicationName("test app")
      .setApplicationVersion(VK_MAKE_VERSION(-1, 0, 0))
      .setPEngineName("test engine")
      .setEngineVersion(VK_MAKE_VERSION(-1, 0, 0))
      .setApiVersion(VK_API_VERSION_1_2);

    const auto instanceCreateInfo = vk::InstanceCreateInfo()
      .setPApplicationInfo(&appInfo)
      .setEnabledExtensionCount(instanceExtensionsCount)
      .setPpEnabledExtensionNames(instanceExtensions)
      .setEnabledLayerCount((uint32_t)validationLayers.size())
      .setPpEnabledLayerNames(validationLayers.data());

    this->instance = vk::createInstanceUnique(instanceCreateInfo);
  }

  this->physicalDevice = GetPhysicalDevice(instance.get());

  //create surface
  const auto surfaceCreateInfo = vk::Win32SurfaceCreateInfoKHR()
    .setHwnd(hWnd)
    .setHinstance(hInstance);

  this->surface = instance->createWin32SurfaceKHRUnique(surfaceCreateInfo);

  //find queue indices
  {
    std::vector<vk::QueueFamilyProperties> queueFamilies = physicalDevice.getQueueFamilyProperties();

    graphicsFamilyIndex = uint32_t(-1);
    presentFamilyIndex = uint32_t(-1);
    transferFamilyIndex = uint32_t(-1);
    for (uint32_t familyIndex = 0; familyIndex < queueFamilies.size(); familyIndex++)
    {
      if (queueFamilies[familyIndex].queueFlags & vk::QueueFlagBits::eGraphics && queueFamilies[familyIndex].queueCount > 0 && graphicsFamilyIndex == uint32_t(-1))
        graphicsFamilyIndex = familyIndex;

      if (physicalDevice.getSurfaceSupportKHR(familyIndex, surface.get()) && queueFamilies[familyIndex].queueCount > 0 && presentFamilyIndex == uint32_t(-1))
        presentFamilyIndex = familyIndex;

      if (queueFamilies[familyIndex].queueFlags & vk::QueueFlagBits::eTransfer && queueFamilies[familyIndex].queueCount > 0 && transferFamilyIndex == uint32_t(-1))
        transferFamilyIndex;
    }
    if (graphicsFamilyIndex == uint32_t(-1) || presentFamilyIndex == uint32_t(-1) || transferFamilyIndex == uint32_t(-1))
      throw std::runtime_error("Failed to find appropriate queue families");

  }

  //create logical device
  {
    std::set<uint32_t> uniqueQueueFamilyIndices = { graphicsFamilyIndex, presentFamilyIndex, transferFamilyIndex };

    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilyIndices) {
      auto queueCreateInfo = vk::DeviceQueueCreateInfo()
        .setQueueFamilyIndex(queueFamily)
        .setQueueCount(1)
        .setPQueuePriorities(&queuePriority);

      queueCreateInfos.push_back(queueCreateInfo);
    }

    vk::PhysicalDeviceFeatures deviceFeatures = {};
    deviceFeatures.fragmentStoresAndAtomics = true;
    deviceFeatures.vertexPipelineStoresAndAtomics = true;

    std::vector<const char*> deviceExtensions;
    deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    const auto deviceCreateInfo = vk::DeviceCreateInfo()
      .setQueueCreateInfoCount(uint32_t(queueCreateInfos.size()))
      .setPQueueCreateInfos(queueCreateInfos.data())
      //.setPEnabledFeatures(&deviceFeatures)
      .setEnabledExtensionCount(uint32_t(deviceExtensions.size()))
      .setPpEnabledExtensionNames(deviceExtensions.data())
      .setEnabledLayerCount((uint32_t)validationLayers.size())
      .setPpEnabledLayerNames(validationLayers.data());

    this->logicalDevice = physicalDevice.createDeviceUnique(deviceCreateInfo);
  }

  //get queues
  this->graphicsQueue = logicalDevice->getQueue(graphicsFamilyIndex, 0);
  this->presentQueue = logicalDevice->getQueue(presentFamilyIndex, 0);
  this->transferQueue = logicalDevice->getQueue(transferFamilyIndex, 0);

  //get memory types index
  hostVisibleMemoryIndex = -1;
  deviceLocalMemoryIndex = -1;
  vk::PhysicalDeviceMemoryProperties memoryProperties = physicalDevice.getMemoryProperties();
  for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i)
  {
    const vk::MemoryType& memType = memoryProperties.memoryTypes[i];
    if ( (hostVisibleMemoryIndex == -1) && memType.propertyFlags & (vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent))
    {
      hostVisibleMemoryIndex = i;
    }

    if ((deviceLocalMemoryIndex == -1) && memType.propertyFlags & vk::MemoryPropertyFlagBits::eDeviceLocal)
    {
      deviceLocalMemoryIndex = i;
    }
  }
  if (hostVisibleMemoryIndex == -1)
    throw std::runtime_error("No host visible memory heap found");

  if (deviceLocalMemoryIndex == -1)
    throw std::runtime_error("No device local memory heap found");

  //create command pool
  const auto cmdPoolCreateInfo = vk::CommandPoolCreateInfo()
    .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
    .setQueueFamilyIndex(graphicsFamilyIndex);
  cmdPool = logicalDevice->createCommandPoolUnique(cmdPoolCreateInfo);

  //create swapchain
  const int swapchainImagesCount = 2;
  swapchain = std::make_unique<Swapchain>(surface.get(), windowSize, *instance, physicalDevice, *logicalDevice, presentFamilyIndex, graphicsFamilyIndex, vk::PresentModeKHR::eImmediate, swapchainImagesCount, presentQueue);

  //create descriptor pool
  std::vector<vk::DescriptorPoolSize> dscPoolSizes;
  dscPoolSizes.push_back(
    vk::DescriptorPoolSize()
    .setDescriptorCount(1000)
    .setType(vk::DescriptorType::eUniformBuffer)
  );

  const auto dscPoolCreateInfo = vk::DescriptorPoolCreateInfo()
    .setMaxSets(1000)
    .setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet)
    .setPoolSizeCount(dscPoolSizes.size())
    .setPPoolSizes(dscPoolSizes.data());

  descriptorPool = logicalDevice->createDescriptorPoolUnique(dscPoolCreateInfo);

  //create storages
  fbStorage = std::make_unique<FramebufferStorage>(logicalDevice.get());
  rpStorage = std::make_unique<RenderPassStorage>(*this);
  ppStorage = std::make_unique<PipelineStorage>(*this);

  //create virtual frames without swapchain image
  currentVirtualFrame = 0;
  for (int i = 0; i < swapchainImagesCount; ++i)
  {
    FrameResources fr;
    fr.cmdBufferFreeToUse = logicalDevice->createFenceUnique(vk::FenceCreateInfo().setFlags(vk::FenceCreateFlagBits::eSignaled));
    fr.swapchainImageAckquired = logicalDevice->createSemaphoreUnique(vk::SemaphoreCreateInfo());
    fr.renderingFinished = logicalDevice->createSemaphoreUnique(vk::SemaphoreCreateInfo());
    fr.uaStorage = std::make_unique<UniformsAccessorStorage>(*this, descriptorPool.get());
    fr.renderGraph = std::make_unique<RenderGraph>(*this);

    const auto cmdBufferAllocateInfo = vk::CommandBufferAllocateInfo()
      .setCommandPool(cmdPool.get())
      .setCommandBufferCount(1)
      .setLevel(vk::CommandBufferLevel::ePrimary);

    fr.cmdBuffer = std::move(logicalDevice->allocateCommandBuffersUnique(cmdBufferAllocateInfo)[0]);

    frameResources.push_back(std::move(fr));
  }

}

FramebufferStorage& Core::GetFramebufferStorage()
{
  return *fbStorage;
}

RenderPassStorage& Core::GetRenderPassStorage()
{
  return *rpStorage;
}

PipelineStorage& Core::GetPipelineStorage()
{
  return *ppStorage;
}

vk::Device Core::GetDebugDevice()
{
  return logicalDevice.get();
}

vk::Format Core::GetDebugSurfaceFormat()
{
  return swapchain->GetImageFormat();
}

Shader Core::CreateShader(const std::string& name, const std::vector<uint32_t>& byteCode)
{
  return Shader(logicalDevice.get(), name, byteCode);
}

RenderGraph* Core::BeginFrame()
{
  currentVirtualFrame = (currentVirtualFrame + 1) % frameResources.size();
  FrameResources& fr = frameResources[currentVirtualFrame];

  logicalDevice->waitForFences(1, &fr.cmdBufferFreeToUse.get(), true, -1);
  logicalDevice->resetFences(1, &fr.cmdBufferFreeToUse.get());
  fr.cmdBuffer->reset(vk::CommandBufferResetFlags());

  fr.swapchainImage = swapchain->AcquireNextImage(fr.swapchainImageAckquired.get());
  fr.uaStorage->Reset();
  fr.renderGraph->Reset();

  const auto bfd = BackbufferDescription()
    .SetFormat(swapchain->GetImageFormat())
    .SetSize(swapchain->GetSurfaceSize());

  fr.renderGraph->SetCommandBuffer(fr.cmdBuffer.get());
  fr.renderGraph->SetUniformsAccessorStorage(fr.uaStorage.get());
  fr.renderGraph->SetBackbufferDescription(bfd);
  fr.renderGraph->AddAttachmentResource(BACKBUFFER_RESOURCE_ID, fr.swapchainImage, ImageUsage::Present);

  return fr.renderGraph.get();
}

void Core::EndFrame()
{
  FrameResources& fr = frameResources[currentVirtualFrame];

  fr.renderGraph->Compile();
  fr.renderGraph->Execute();

  vk::PipelineStageFlags stageFlags[]{
    vk::PipelineStageFlagBits::eAllGraphics
  };

  const vk::SubmitInfo submitInfo[] = {
    vk::SubmitInfo()
    .setCommandBufferCount(1)
    .setPCommandBuffers(&fr.cmdBuffer.get())
    .setWaitSemaphoreCount(1)
    .setPWaitSemaphores(&fr.swapchainImageAckquired.get())
    .setPWaitDstStageMask(stageFlags)
    .setSignalSemaphoreCount(1)
    .setPSignalSemaphores(&fr.renderingFinished.get())
  };

  //in fact, we are only waiting when swapchain image will be free to use, it is not about the cmd buffer itself!
  graphicsQueue.submit(1, submitInfo, fr.cmdBufferFreeToUse.get());

  swapchain->PresentImage(fr.renderingFinished.get());
}

HostBuffer Core::AllocateHostBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage)
{
  auto [hostBuffer, bufferMemory, memSize] = AllocateBuffer(size, usage, graphicsFamilyIndex, hostVisibleMemoryIndex);

  return HostBuffer{ logicalDevice.get(), std::move(hostBuffer), std::move(bufferMemory), memSize };
}

Buffer Core::AllocateDeviceBuffer(void* src, vk::DeviceSize size, vk::BufferUsageFlags usage)
{
  HostBuffer hostBuffer = AllocateHostBuffer(size, usage | vk::BufferUsageFlagBits::eTransferSrc);
  hostBuffer.UploadMemory(src, size, 0);

  auto [deviceBuffer, deviceBufferMemory, deviceBufferMemSize] = AllocateBuffer(size, usage | vk::BufferUsageFlagBits::eTransferDst, transferFamilyIndex, deviceLocalMemoryIndex);

  const auto cmdBufferAllocateInfo = vk::CommandBufferAllocateInfo()
    .setCommandPool(cmdPool.get())
    .setCommandBufferCount(1)
    .setLevel(vk::CommandBufferLevel::ePrimary);

  vk::UniqueCommandBuffer cmdBuffer = std::move(logicalDevice->allocateCommandBuffersUnique(cmdBufferAllocateInfo)[0]);
  cmdBuffer->begin(vk::CommandBufferBeginInfo());

  const auto copyRegion = vk::BufferCopy()
    .setSize(deviceBufferMemSize)
    .setSrcOffset(0)
    .setDstOffset(0);

  cmdBuffer->copyBuffer(hostBuffer.GetBuffer(), deviceBuffer.get(), 1, &copyRegion);

  vk::UniqueFence bufferCopiedFence = logicalDevice->createFenceUnique(vk::FenceCreateInfo().setFlags(vk::FenceCreateFlagBits::eSignaled));
  const auto submitInfo = vk::SubmitInfo()
    .setCommandBufferCount(1)
    .setPCommandBuffers(&cmdBuffer.get());

  transferQueue.submit(1, &submitInfo, bufferCopiedFence.get());

  const bool waitAll = true;
  logicalDevice->waitForFences(1, &bufferCopiedFence.get(), waitAll, uint64_t(-1));

  return Buffer{ logicalDevice.get(), std::move(deviceBuffer), std::move(deviceBufferMemory), deviceBufferMemSize };
}

std::tuple<vk::UniqueBuffer, vk::UniqueDeviceMemory, vk::DeviceSize> Core::AllocateBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, uint32_t queueFamilyIndex, uint32_t memoryTypeIndex)
{
  const auto bufferCreateInfo = vk::BufferCreateInfo()
    //.setFlags()
    .setSize(size)
    .setUsage(usage)
    .setSharingMode(vk::SharingMode::eExclusive)
    .setQueueFamilyIndexCount(1)
    .setPQueueFamilyIndices(&queueFamilyIndex);

  vk::UniqueBuffer buf = logicalDevice->createBufferUnique(bufferCreateInfo);

  const vk::MemoryRequirements memRec = logicalDevice->getBufferMemoryRequirements(buf.get());

  const auto memAllocateInfo = vk::MemoryAllocateInfo()
    .setAllocationSize(memRec.size)
    .setMemoryTypeIndex(memoryTypeIndex);

  vk::UniqueDeviceMemory memory = logicalDevice->allocateMemoryUnique(memAllocateInfo);

  logicalDevice->bindBufferMemory(buf.get(), memory.get(), 0);

  return { std::move(buf), std::move(memory), memRec.size };
}

Image Core::AllocateImage(vk::ImageType type, vk::Format format, const vk::Extent3D& extent, vk::ImageUsageFlags usage, vk::ImageAspectFlags aspectMask)
{
  const auto imageCreateInfo = vk::ImageCreateInfo()
    //.setFlags()
    .setImageType(type)
    .setFormat(format)
    .setExtent(extent)
    .setMipLevels(1)
    .setArrayLayers(1)
    .setSamples(vk::SampleCountFlagBits::e1)
    .setTiling(vk::ImageTiling::eOptimal)
    .setUsage(usage)
    .setSharingMode(vk::SharingMode::eExclusive)
    .setInitialLayout(vk::ImageLayout::eUndefined);

  vk::UniqueImage img = logicalDevice->createImageUnique(imageCreateInfo);

  const vk::MemoryRequirements memRec = logicalDevice->getImageMemoryRequirements(img.get());

  const auto memAllocateInfo = vk::MemoryAllocateInfo()
    .setAllocationSize(memRec.size)
    .setMemoryTypeIndex(deviceLocalMemoryIndex);

  vk::UniqueDeviceMemory memory = logicalDevice->allocateMemoryUnique(memAllocateInfo);

  logicalDevice->bindImageMemory(img.get(), memory.get(), 0);

  const auto subresourceRange = vk::ImageSubresourceRange()
    .setAspectMask(aspectMask)
    .setBaseMipLevel(0)
    .setLevelCount(1)
    .setBaseArrayLayer(0)
    .setLayerCount(1);

  const auto viewCreateInfo = vk::ImageViewCreateInfo()
    .setImage(img.get())
    .setViewType(vk::ImageViewType::e2D)
    .setFormat(format)
    .setSubresourceRange(subresourceRange);

  vk::UniqueImageView view = logicalDevice->createImageViewUnique(viewCreateInfo);

  return Image{ std::move(img), std::move(view), std::move(memory) };
}

Image Core::Allocate2DImage(vk::Format format, vk::Extent2D extent, vk::ImageUsageFlags usage)
{
  return AllocateImage(vk::ImageType::e2D, format, vk::Extent3D{ extent.width, extent.height, 1 }, usage, vk::ImageAspectFlagBits::eColor);
}

Image Core::AllocateDepthStencilImage(vk::Format format, vk::Extent2D extent)
{
  return AllocateImage(vk::ImageType::e2D, format, vk::Extent3D{ extent.width, extent.height, 1 }, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil);
}

vk::PhysicalDevice Core::GetPhysicalDevice(vk::Instance instance)
{
  std::vector<vk::PhysicalDevice> physicalDevices = instance.enumeratePhysicalDevices();

  vk::PhysicalDevice physicalDevice = nullptr;
  for (const auto& device : physicalDevices)
  {
    vk::PhysicalDeviceProperties deviceProperties = device.getProperties();

    if (deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
    {
      physicalDevice = device;
    }
  }
  if (!physicalDevice)
    throw std::runtime_error("Failed to find physical device");

  return physicalDevice;
}

vk::Device Core::GetLogicalDevice() const
{
  return logicalDevice.get();
}