#pragma once

#include "enums.h"

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>

#include <map>
#include <vector>
#include <tuple>
#include <memory>

class Core;

struct SubpassKey
{
  bool operator<(const SubpassKey& r) const;

  std::vector<vk::AttachmentReference> inputAttachmentReferences;
  std::vector<vk::AttachmentReference> outputColorAttachmentReferences;
};

class RenderPassKey
{
  friend class RenderPassStorage;
public:
  RenderPassKey& SetSubpassesDescriptions(const std::vector<SubpassKey>& descs);

  RenderPassKey& SetDependencies(const std::vector<vk::SubpassDependency>& deps);

  RenderPassKey& SetAttachmentsUsages(const std::vector<ImageUsage>& u);

  RenderPassKey& SetBackbufferFormat(const vk::Format& f);

  bool operator<(const RenderPassKey& r) const;

private:
  std::vector<SubpassKey> subpassesDescriptions;
  std::vector<vk::SubpassDependency> dependencies;
  std::vector<ImageUsage> AttachmentsUsages;
  vk::Format backbufferFormat;
};

class RenderPassStorage
{
public:
  RenderPassStorage(Core& core);

  vk::RenderPass GetRenderPass(const RenderPassKey& key);

private:
  Core& core;
  std::map<RenderPassKey, vk::UniqueRenderPass> renderPasses;
};