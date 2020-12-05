#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>
#include <memory>

class Buffer
{
public:
  Buffer(vk::Device logicalDevice, vk::UniqueBuffer&& buffer, vk::UniqueDeviceMemory&& memory, vk::DeviceSize memorySize)
    : logicalDevice(logicalDevice)
    , buffer(std::move(buffer))
    , memory(std::move(memory))
    , mappedMemory(nullptr)
    , memorySize(memorySize)
  {
    fullBufferUpdateInfo = vk::DescriptorBufferInfo()
      .setBuffer(this->buffer.get())
      .setOffset(0)
      .setRange(VK_WHOLE_SIZE);
  }

  void* Map()
  {
    mappedMemory = logicalDevice.mapMemory(memory.get(), 0, memorySize);
    return mappedMemory;
  }

  void Unmap()
  {
    logicalDevice.unmapMemory(memory.get());
    mappedMemory = nullptr;
  }

  void UploadMemory(const void* src, vk::DeviceSize size, vk::DeviceSize offset)
  {
    mappedMemory = logicalDevice.mapMemory(memory.get(), offset, size);

    std::memcpy(mappedMemory, src, size);

    logicalDevice.unmapMemory(memory.get());
    mappedMemory = nullptr;
  }

  void* GetMappedMemory() const
  {
    return mappedMemory;
  }

  vk::Buffer GetBuffer() const 
  {
    return buffer.get();
  }

  inline const vk::DescriptorBufferInfo& GetFullBufferUpdateInfo() const
  {
    return fullBufferUpdateInfo;
  }

private:
  vk::Device logicalDevice;
  vk::UniqueBuffer buffer;
  vk::UniqueDeviceMemory memory;
  void* mappedMemory;
  vk::DeviceSize memorySize;

  vk::DescriptorBufferInfo fullBufferUpdateInfo;
};