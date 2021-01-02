#include "framegraph.h"
#include "core.h"

RenderSubpass::RenderSubpass(unsigned int id)
  : id(id)
{
}

RenderSubpass& RenderSubpass::AddInputAttachment(const InputAttachmentDescription& desc)
{
  inputAttachments.push_back(desc);

  return *this;
}

RenderSubpass& RenderSubpass::AddInputSampler()
{
  return *this;
}

RenderSubpass& RenderSubpass::AddInputBuffer()
{
  return *this;
}

RenderSubpass& RenderSubpass::AddOutputColorAttachment(const ImageAttachmentDescription& desc)
{
  outputColorAttachments.push_back(OutputColorAttachmentDescription{ desc.id });
  imageResourceDescriptions.push_back(desc);

  return *this;
}

RenderSubpass& RenderSubpass::AddOutputColorAttachment(const OutputColorAttachmentDescription& desc)
{
  outputColorAttachments.push_back(desc);

  return *this;
}

RenderSubpass& RenderSubpass::AddDepthStencilAttachment(const DepthStencilAttachmentDescription& desc)
{
  depthStencilAttachment = desc;

  return *this;
}

RenderSubpass& RenderSubpass::AddOutputBuffer()
{
  return *this;
}

RenderSubpass& RenderSubpass::AddOutputSampler()
{
  return *this;
}

RenderSubpass& RenderSubpass::SetRenderCallback(RenderPassExecutionFunction callback)
{
  renderCallback = callback;
  return *this;
}

RenderGraph::RenderGraph(Core& core)
  : core(core)
{
}

RenderSubpass& RenderGraph::AddRenderSubpass()
{
  const unsigned id = static_cast<unsigned int>(subpasses.size());
  subpasses.push_back(RenderSubpass{ id });

  return subpasses.back();
}

void RenderGraph::Compile()
{
  AllocateSubpassesResources();

  this->renderPass = CreateRenderpass();
  this->framebuffer = CreateFramebuffer();
}

std::vector<vk::SubpassDependency> RenderGraph::GetAttachmentDependencies()
{
  std::vector<vk::SubpassDependency> deps;

  std::map<ResourceId, SubpassId> ResourceIdToSubpassProducerMap;
  for (const RenderSubpass& subpass : subpasses)
  {
    for (const OutputColorAttachmentDescription outputAttachmentDescription : subpass.outputColorAttachments)
    {
      ResourceIdToSubpassProducerMap[outputAttachmentDescription.id] = subpass.id;
    }
  }

  for (const RenderSubpass& subpass : subpasses)
  {
    std::set<SubpassId> parents;
    for (const InputAttachmentDescription& inputAttachmentDescription : subpass.inputAttachments)
    {
      parents.insert(ResourceIdToSubpassProducerMap[inputAttachmentDescription.id]);
    }

    for (SubpassId parentId : parents)
    {
      const SubpassId src = subpass.id < parentId ? subpass.id : parentId;
      const SubpassId dst = subpass.id > parentId ? subpass.id : parentId;

      const auto dep = vk::SubpassDependency()
        .setSrcSubpass(src)
        .setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
        .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
        .setDstSubpass(dst)
        .setDstAccessMask(vk::AccessFlagBits::eInputAttachmentRead)
        .setDstStageMask(vk::PipelineStageFlagBits::eFragmentShader);

      deps.push_back(dep);
    }
  }

  return deps;
}

void RenderGraph::AddAttachmentResource(ResourceId id, vk::ImageView view, ImageUsage usage)
{
  if (resourceIdToAttachmentIdMap.find(id) != resourceIdToAttachmentIdMap.end())
    throw std::runtime_error("AddAttachmentResource: resource with such id already exist");

  const AttachmentId attId = static_cast<AttachmentId>(imageAttachmentResources.size());
  imageAttachmentResources.push_back(view);
  imageAttachmentResourceUsages.push_back(usage);
  resourceIdToAttachmentIdMap[id] = attId;
}

void RenderGraph::SetBackbufferDescription(const BackbufferDescription& bfDescription)
{
  backbufferDescription = bfDescription;
}

void RenderGraph::SetCommandBuffer(vk::CommandBuffer cmdBuf)
{
  cmdBuffer = cmdBuf;
}

void RenderGraph::SetUniformsAccessorStorage(UniformsAccessorStorage* s)
{
  uaStorage = s;
}

void RenderGraph::Reset()
{
  subpasses.clear();
  resourceIdToAttachmentIdMap.clear();
  imageAttachmentResources.clear();
  imageAttachmentResourceUsages.clear();
  ownedImages.clear();
}

void RenderGraph::Execute()
{
  std::vector<vk::ClearValue> clearColors;
  for (const auto& usage : imageAttachmentResourceUsages)
  {
    vk::ClearValue clearValue;

    if (usage == ImageUsage::DepthStencil)
      clearValue = vk::ClearDepthStencilValue(1.0f, 0.0f);
    else
      clearValue = vk::ClearColorValue{ std::array<float,4>{ 0.5529f, 0.6f, 0.6823f, 1.0f} };

    clearColors.push_back(clearValue);
  }

  const auto rpBeginInfo = vk::RenderPassBeginInfo()
    .setRenderPass(renderPass)
    .setFramebuffer(framebuffer)
    .setRenderArea(vk::Rect2D{ {0,0}, backbufferDescription.size })
    .setClearValueCount(clearColors.size())
    .setPClearValues(clearColors.data());

  cmdBuffer.begin(vk::CommandBufferBeginInfo());
  cmdBuffer.beginRenderPass(rpBeginInfo, vk::SubpassContents::eInline);

  FrameContext context;
  context.BackbufferSize = backbufferDescription.size;
  context.uniformsAccessorStorage = uaStorage;
  context.pipelineStorage = &core.GetPipelineStorage();
  context.framebuffer = framebuffer;
  context.renderPass = renderPass;
  context.commandBuffer = cmdBuffer;
  
  for (int i = 0; i < subpasses.size(); ++i)
  {
    const RenderSubpass& subpass = subpasses[i];

    context.subpassNumber = subpass.id;
    subpass.renderCallback(context);

    if (i != (subpasses.size() - 1))
      cmdBuffer.nextSubpass(vk::SubpassContents::eInline);
  }

  cmdBuffer.endRenderPass();
  cmdBuffer.end();
}

vk::RenderPass RenderGraph::CreateRenderpass()
{
  //describe subbpasses and it's attachment references
  std::vector<SubpassKey> subpassKeys;
  for (const RenderSubpass& subpass : subpasses)
  {
    SubpassKey subkey;

    for (const InputAttachmentDescription& inputDesc : subpass.inputAttachments)
    {
      AttachmentId fbId = resourceIdToAttachmentIdMap[inputDesc.id];
      subkey.inputAttachmentReferences.push_back(
        vk::AttachmentReference()
        .setAttachment(fbId)
        .setLayout(inputDesc.layout)
      );
    }

    for (const OutputColorAttachmentDescription& outputDesc : subpass.outputColorAttachments)
    {
      AttachmentId fbId = resourceIdToAttachmentIdMap[outputDesc.id];
      subkey.outputColorAttachmentReferences.push_back(
        vk::AttachmentReference()
        .setAttachment(fbId)
        .setLayout(vk::ImageLayout::eColorAttachmentOptimal)
      );
    }

    if (subpass.depthStencilAttachment.has_value())
    {
      const DepthStencilAttachmentDescription& desc = subpass.depthStencilAttachment.value();
      AttachmentId fbId = resourceIdToAttachmentIdMap[desc.id];
      subkey.depthStencilAttachmentReference = vk::AttachmentReference()
        .setAttachment(fbId)
        .setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
    }

    subpassKeys.push_back(std::move(subkey));
  }

  std::vector<vk::SubpassDependency> deps = GetAttachmentDependencies();

  const auto rpKey = RenderPassKey()
    .SetBackbufferFormat(backbufferDescription.format)
    .SetDependencies(deps)
    .SetSubpassesDescriptions(subpassKeys)
    .SetAttachmentsUsages(imageAttachmentResourceUsages);

  return core.GetRenderPassStorage().GetRenderPass(rpKey);
}

vk::Framebuffer RenderGraph::CreateFramebuffer()
{
  const auto fbKey = FramebufferKey()
    .SetAttachments(imageAttachmentResources)
    .SetHeight(backbufferDescription.size.height)
    .SetWidth(backbufferDescription.size.width)
    .SetLayers(1)
    .SetRenderPass(renderPass);

  return core.GetFramebufferStorage().GetFramebuffer(fbKey);
}

void RenderGraph::AllocateSubpassesResources()
{
  for (const RenderSubpass& subpass : subpasses)
  {
    for (const ImageAttachmentDescription& desc : subpass.imageResourceDescriptions)
    {
      if (resourceIdToAttachmentIdMap.find(desc.id) != resourceIdToAttachmentIdMap.end())
        throw std::runtime_error("AllocateSubpassesResources: can't add a new resource: index already in use.");

      Image img = core.Allocate2DImage(backbufferDescription.format, backbufferDescription.size, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eInputAttachment);

      const AttachmentId attId = static_cast<AttachmentId>(imageAttachmentResources.size());
      imageAttachmentResources.push_back(img.GetView());
      imageAttachmentResourceUsages.push_back(ImageUsage::Default);
      resourceIdToAttachmentIdMap[desc.id] = attId;

      ownedImages.push_back(std::move(img));
    }

    if (subpass.depthStencilAttachment.has_value())
    {
      const DepthStencilAttachmentDescription& desc = subpass.depthStencilAttachment.value();

      if (resourceIdToAttachmentIdMap.find(desc.id) != resourceIdToAttachmentIdMap.end())
        throw std::runtime_error("AllocateSubpassesResources: can't add a new depth|stencil resource: index already in use.");

      Image img = core.AllocateDepthStencilImage(vk::Format::eD32SfloatS8Uint, backbufferDescription.size);

      const AttachmentId attId = static_cast<AttachmentId>(imageAttachmentResources.size());
      imageAttachmentResources.push_back(img.GetView());
      imageAttachmentResourceUsages.push_back(ImageUsage::DepthStencil);
      resourceIdToAttachmentIdMap[desc.id] = attId;

      ownedImages.push_back(std::move(img));
    }
  }
}