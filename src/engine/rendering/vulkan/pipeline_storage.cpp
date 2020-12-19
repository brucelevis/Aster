#include "pipeline_storage.h"
#include "core.h"
#include "framecontext.h"

namespace vk
{
  bool operator<(const vk::Extent2D& l, const vk::Extent2D& r)
  {
    return std::tie(l.height, l.width) < std::tie(r.height, r.width);
  }
}

PipelineKey& PipelineKey::SetVertexShaderName(const std::string& s)
{
  vertexShader = s;
  return *this;
}

PipelineKey& PipelineKey::SetFragmentShaderName(const std::string s)
{
  fragmentShader = s;
  return *this;
}

PipelineKey& PipelineKey::SetVertexInputDeclaration(const VertexInputDeclaration& d)
{
  vertexInputDeclaration = d;
  return *this;
}

PipelineKey& PipelineKey::SetTopology(const vk::PrimitiveTopology t)
{
  topology = t;
  return *this;
}

PipelineKey& PipelineKey::SetDepthStencilSettings(const DepthStencilSettings& s)
{
  depthStencilSettings = s;
  return *this;
}

PipelineKey& PipelineKey::SetViewportExtent(const vk::Extent2D e)
{
  viewportExtent = e;
  return *this;
}

PipelineKey& PipelineKey::SetRenderPass(const vk::RenderPass r)
{
  renderpass = r;
  return *this;
}

PipelineKey& PipelineKey::SetSubpassNumber(const uint32_t n)
{
  subpass = n;
  return *this;
}

bool PipelineKey::operator<(const PipelineKey& r) const
{
  return std::tie(vertexShader, fragmentShader, vertexInputDeclaration, topology, viewportExtent, renderpass, subpass) <
    std::tie(r.vertexShader, r.fragmentShader, r.vertexInputDeclaration, r.topology, r.viewportExtent, r.renderpass, r.subpass);
}

PipelineStorage::PipelineStorage(Core& core)
  : core(core)
{
}

Pipeline* PipelineStorage::GetPipeline(const ShaderProgram& program, const VertexInputDeclaration& vertexInputDeclaration, vk::PrimitiveTopology topology, const DepthStencilSettings& depthStencilSettings, const vk::Extent2D& viewportExtent, vk::RenderPass renderPass, uint32_t subpassNumber)
{
  const auto key = PipelineKey()
    .SetVertexShaderName(program.GetVertexShader().GetName())
    .SetFragmentShaderName(program.GetFragmentShader().GetName())
    .SetVertexInputDeclaration(vertexInputDeclaration)
    .SetTopology(topology)
    .SetDepthStencilSettings(depthStencilSettings)
    .SetViewportExtent(viewportExtent)
    .SetRenderPass(renderPass)
    .SetSubpassNumber(subpassNumber);

  if (storage.find(key) != storage.end())
    return storage[key].get();

  const std::vector<vk::DescriptorSetLayout>& layouts = program.GetLayouts();

  std::unique_ptr<Pipeline> pp = std::make_unique<Pipeline>(core.GetLogicalDevice(), program, vertexInputDeclaration, layouts, topology, depthStencilSettings, viewportExtent, renderPass, subpassNumber);
  Pipeline* pipeline = pp.get();
  storage[key] = std::move(pp);

  return pipeline;
}

Pipeline* PipelineStorage::GetPipeline(const ShaderProgram& program, const VertexInputDeclaration& vertexInputDeclaration, vk::PrimitiveTopology topology, const DepthStencilSettings& depthStencilSettings, const FrameContext& frameContext)
{
  return GetPipeline(program, vertexInputDeclaration, topology, depthStencilSettings, frameContext.BackbufferSize, frameContext.renderPass, frameContext.subpassNumber);
}