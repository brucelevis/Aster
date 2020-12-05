#pragma once

#include "image.h"
#include "enums.h"
#include "framecontext.h"

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>

#include <vector>
#include <queue>
#include <map>
#include <set>

class Core;
class PipelineStorage;
class FramebufferStorage;
class RenderPassStorage;
class UniformsContextStorage;

typedef std::string ResourceId;
typedef unsigned int SubpassId;
typedef unsigned int AttachmentId;

#define BACKBUFFER_RESOURCE_ID ResourceId("__backbuffer_resource")

struct ImageAttachmentDescription
{
  ResourceId id;
};

struct InputAttachmentDescription
{
  ResourceId id;
  vk::ImageLayout layout;
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

  RenderSubpass& AddInputAttachment(const InputAttachmentDescription& desc);

  RenderSubpass& AddInputSampler();

  RenderSubpass& AddInputBuffer();

  RenderSubpass& AddOutputColorAttachment(const ImageAttachmentDescription& desc);

  RenderSubpass& AddOutputColorAttachment(const OutputColorAttachmentDescription& desc);

  RenderSubpass& AddOutputBuffer();

  RenderSubpass& AddOutputSampler();

  RenderSubpass& SetRenderCallback(RenderPassExecutionFunction callback);

private:
  const unsigned int id;
  RenderPassExecutionFunction renderCallback;
  std::vector<InputAttachmentDescription> inputAttachments;
  std::vector<OutputColorAttachmentDescription> outputColorAttachments;

  std::vector<ImageAttachmentDescription> imageResourceDescriptions;
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

  void AddAttachmentResource(ResourceId id, vk::ImageView view, ImageUsage usage);

  void SetBackbufferDescription(const BackbufferDescription& bfDescription);

  void SetCommandBuffer(vk::CommandBuffer cmdBuf);

  void SetUniformsContextStorage(UniformsContextStorage* s);

  void Reset();

  void Execute();
private:
  std::vector<vk::SubpassDependency> GetAttachmentDependencies();

  vk::RenderPass CreateRenderpass();

  vk::Framebuffer CreateFramebuffer();

  void AllocateSubpassesResources();

private:
  Core& core;

  UniformsContextStorage* ucStorage;
  vk::CommandBuffer cmdBuffer;
  BackbufferDescription backbufferDescription;
  std::vector<RenderSubpass> subpasses;

  std::map<ResourceId, AttachmentId> resourceIdToAttachmentIdMap;
  std::vector<vk::ImageView> imageAttachmentResources;
  std::vector<ImageUsage> imageAttachmentResourceUsages;

  std::vector<Image> ownedImages;

  vk::RenderPass renderPass;
  vk::Framebuffer framebuffer;
};

