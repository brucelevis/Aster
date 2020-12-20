#include "buffer.h"

Buffer::Buffer(vk::Device logicalDevice, vk::UniqueBuffer&& buffer, vk::UniqueDeviceMemory&& memory, vk::DeviceSize memorySize)
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


void Buffer::UploadMemory(const void* src, vk::DeviceSize size, vk::DeviceSize offset)
{
  mappedMemory = logicalDevice.mapMemory(memory.get(), offset, size);

  std::memcpy(mappedMemory, src, size);

  logicalDevice.unmapMemory(memory.get());
  mappedMemory = nullptr;
}
