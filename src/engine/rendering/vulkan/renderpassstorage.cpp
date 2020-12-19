#include "renderpassstorage.h"
#include "core.h"

namespace vk
{
  bool operator<(const vk::AttachmentReference& l, const vk::AttachmentReference& r)
  {
    return std::tie(l.attachment, l.layout) < std::tie(l.attachment, l.layout);
  }

  bool operator<(const vk::SubpassDependency& l, const vk::SubpassDependency& r)
  {
    return std::tie(l.srcSubpass, l.srcStageMask, l.srcAccessMask, l.dstSubpass, l.dstAccessMask, l.dstStageMask, l.dependencyFlags) <
           std::tie(r.srcSubpass, r.srcStageMask, r.srcAccessMask, r.dstSubpass, r.dstAccessMask, r.dstStageMask, r.dependencyFlags);
  }
}

namespace
{
  vk::ImageLayout GetLayoutFromUsage(ImageUsage u)
  {
    switch (u)
    {
    case ImageUsage::Present:
      return vk::ImageLayout::ePresentSrcKHR;

    case ImageUsage::DepthStencil:
      return vk::ImageLayout::eDepthStencilAttachmentOptimal;

    default:
      return vk::ImageLayout::eColorAttachmentOptimal;
    }
  }
}

bool SubpassKey::operator<(const SubpassKey& r) const
{
  return std::tie(inputAttachmentReferences, outputColorAttachmentReferences) <
    std::tie(r.inputAttachmentReferences, r.outputColorAttachmentReferences);
}

RenderPassKey& RenderPassKey::SetSubpassesDescriptions(const std::vector<SubpassKey>& descs)
{
  subpassesDescriptions = descs;
  return *this;
}

RenderPassKey& RenderPassKey::SetDependencies(const std::vector<vk::SubpassDependency>& deps)
{
  dependencies = deps;
  return *this;
}

RenderPassKey& RenderPassKey::SetAttachmentsUsages(const std::vector<ImageUsage>& u)
{
  AttachmentsUsages = u;
  return *this;
}



RenderPassKey& RenderPassKey::SetBackbufferFormat(const vk::Format& f)
{
  backbufferFormat = f;
  return *this;
}

bool RenderPassKey::operator<(const RenderPassKey& r) const
{
  return std::tie(subpassesDescriptions, dependencies, AttachmentsUsages, backbufferFormat) < std::tie(r.subpassesDescriptions, r.dependencies, r.AttachmentsUsages, r.backbufferFormat);
}

RenderPassStorage::RenderPassStorage(Core& core)
  : core(core)
{
}

vk::RenderPass RenderPassStorage::GetRenderPass(const RenderPassKey& key)
{
  if (renderPasses.find(key) != renderPasses.end())
  {
    return renderPasses[key].get();
  }

  std::vector<vk::AttachmentDescription> attachmentDescriptions;
  for (int i = 0; i < key.AttachmentsUsages.size(); ++i)
  {
    const vk::ImageLayout finalLayout = ::GetLayoutFromUsage(key.AttachmentsUsages[i]);
    const vk::Format format = key.AttachmentsUsages[i] == ImageUsage::DepthStencil ? vk::Format::eD32SfloatS8Uint : key.backbufferFormat;
    const vk::AttachmentLoadOp lOp = (key.AttachmentsUsages[i] == ImageUsage::Present) || (key.AttachmentsUsages[i] == ImageUsage::DepthStencil) ? vk::AttachmentLoadOp::eClear : vk::AttachmentLoadOp::eDontCare;

    const auto desc = vk::AttachmentDescription()
      .setFormat(format)
      .setSamples(vk::SampleCountFlagBits::e1)
      .setLoadOp(lOp)
      .setStoreOp(vk::AttachmentStoreOp::eStore)
      .setInitialLayout(vk::ImageLayout::eUndefined)
      .setFinalLayout(finalLayout);

    attachmentDescriptions.push_back(desc);
  }

  std::vector<vk::SubpassDescription> subpassesDescriptions;
  for (const SubpassKey& sk : key.subpassesDescriptions)
  {
    auto sd = vk::SubpassDescription()
      .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
      .setInputAttachmentCount(static_cast<uint32_t>(sk.inputAttachmentReferences.size()))
      .setPInputAttachments(sk.inputAttachmentReferences.data())
      .setColorAttachmentCount(static_cast<uint32_t>(sk.outputColorAttachmentReferences.size()))
      .setPColorAttachments(sk.outputColorAttachmentReferences.data());

    if (sk.depthStencilAttachmentReference.has_value())
      sd.setPDepthStencilAttachment(&sk.depthStencilAttachmentReference.value());

    subpassesDescriptions.push_back(std::move(sd));
  }

  const auto rpCreateInfo = vk::RenderPassCreateInfo()
    .setAttachmentCount(static_cast<uint32_t>(attachmentDescriptions.size()))
    .setPAttachments(attachmentDescriptions.data())
    .setSubpassCount(static_cast<uint32_t>(subpassesDescriptions.size()))
    .setPSubpasses(subpassesDescriptions.data())
    .setDependencyCount(static_cast<uint32_t>(key.dependencies.size()))
    .setPDependencies(key.dependencies.data());

  vk::UniqueRenderPass rpUnique = core.GetLogicalDevice().createRenderPassUnique(rpCreateInfo);
  vk::RenderPass rp = rpUnique.get();
  renderPasses[key] = std::move(rpUnique);

  return rp;
}