#pragma once

#include "enums.h"

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>

#include <tuple>

typedef std::string ResourceId;

struct ImageAttachment
{
  ResourceId id;
  vk::Format format;
  vk::ImageView view;
  ImageType type;
  vk::ImageLayout initialLayout;
  vk::ImageLayout finalLayout;
  vk::AttachmentLoadOp loadOp;
  vk::AttachmentStoreOp storeOp;
  vk::ImageUsageFlags usageFlags;

  inline bool operator<(const ImageAttachment& r) const
  {
    return std::tie(id, format, view, type, initialLayout, finalLayout, loadOp, storeOp, usageFlags) <
           std::tie(r.id, r.format, r.view, r.type, r.initialLayout, r.finalLayout, r.loadOp, r.storeOp, r.usageFlags);
  }
};
