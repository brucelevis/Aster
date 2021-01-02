#pragma once

#include "common.h"
#include "image.h"
#include "enums.h"
#include "framecontext.h"

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>

#include <vector>
#include <queue>
#include <map>
#include <set>
#include <optional>

class Core;
class PipelineStorage;
class FramebufferStorage;
class RenderPassStorage;
class UniformsAccessorStorage;

typedef unsigned int SubpassId;
typedef unsigned int AttachmentId;

#define BACKBUFFER_RESOURCE_ID ResourceId("__backbuffer_resource")

struct InputAttachment
{
  ResourceId id;
  vk::ImageLayout layout;
};

struct DepthStencilAttachmentDescription
{
  ResourceId id;
};

struct OutputColorAttachmentDescription
{
  ResourceId id;
};

class ITask
{
public:
  virtual void Execute() = 0;
};

typedef std::function<void(FrameContext&)> RenderPassExecutionFunction;

// Rendering task
// describes a unique subpass of Render Pass
class RenderSubpass
{
  friend class RenderGraph;
public:
  RenderSubpass(unsigned int id);

  RenderSubpass& AddInputAttachment(const InputAttachment& desc);

  RenderSubpass& AddInputSampler();

  RenderSubpass& AddInputBuffer();

  RenderSubpass& AddNewOutputColorAttachment(const ResourceId& id, vk::Format format = vk::Format::eUndefined);

  RenderSubpass& AddExistOutputColorAttachment(const ResourceId& id);

  RenderSubpass& AddDepthOnlyAttachment(const ResourceId& id);

  RenderSubpass& AddDepthStencilAttachment(const ResourceId& id);

  RenderSubpass& AddOutputBuffer();

  RenderSubpass& AddOutputSampler();

  RenderSubpass& SetRenderCallback(RenderPassExecutionFunction callback);

private:
  const unsigned int id;
  RenderPassExecutionFunction renderCallback;
  std::vector<InputAttachment> inputAttachments;
  std::vector<ResourceId> outputColorAttachments;

  std::vector<ImageAttachment> imageAttachmentCreateInfos;
  std::optional<ImageAttachment> depthStencilAttachment;
};

struct BackbufferDescription
{
  vk::Format format;
  vk::Extent2D size;

  BackbufferDescription& SetFormat(vk::Format f)
  {
    format = f;
    return *this;
  }

  BackbufferDescription& SetSize(const vk::Extent2D& s)
  {
    size = s;
    return *this;
  }
};

class RenderGraph
{
public:
  RenderGraph(Core& core);

  RenderSubpass& AddRenderSubpass();

  void Compile();

  void AddAttachmentResource(const ImageAttachment& attachment);

  void SetBackbufferDescription(const BackbufferDescription& bfDescription);

  void SetCommandBuffer(vk::CommandBuffer cmdBuf);

  void SetUniformsAccessorStorage(UniformsAccessorStorage* s);

  void Reset();

  void Execute();
private:
  std::vector<vk::SubpassDependency> GetAttachmentDependencies();

  vk::RenderPass CreateRenderpass();

  vk::Framebuffer CreateFramebuffer();

  void AllocateSubpassesResources();

private:
  Core& core;

  UniformsAccessorStorage* uaStorage;
  vk::CommandBuffer cmdBuffer;
  BackbufferDescription backbufferDescription;
  std::vector<RenderSubpass> subpasses;

  std::map<ResourceId, AttachmentId> resourceIdToAttachmentIdMap;
  std::vector<ImageAttachment> imageAttachments;

  std::vector<Image> ownedImages;

  vk::RenderPass renderPass;
  vk::Framebuffer framebuffer;
};

