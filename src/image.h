#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>

class Image
{
public:

  Image(const vk::Device& logicalDevice, const vk::Image& img, vk::Format format, vk::Extent2D extent)
  {
    this->swapchainImage = img;

    const auto subresourceRange = vk::ImageSubresourceRange()
      .setAspectMask(vk::ImageAspectFlagBits::eColor)
      .setBaseMipLevel(0)
      .setLevelCount(1)
      .setBaseArrayLayer(0)
      .setLayerCount(1);

    const auto viewCreateInfo = vk::ImageViewCreateInfo()
      .setImage(img)
      .setViewType(vk::ImageViewType::e2D)
      .setFormat(format)
      .setSubresourceRange(subresourceRange);

    view = logicalDevice.createImageViewUnique(viewCreateInfo);
  }

  Image(vk::UniqueImage img, vk::UniqueImageView view)
  {
    image = std::move(img);
    view = std::move(view);
  }

  vk::ImageView GetView() const
  {
    return *view;
  }

private:
  vk::Image swapchainImage;
  vk::UniqueImage image;
  vk::UniqueImageView view;
};