#include "uniformscontext.h"
#include "core.h"

UniformsContext::UniformsContext(Core& core, vk::DescriptorPool descriptorPool, const std::vector<vk::DescriptorSetLayout>& layouts, const PipelineUniforms& uniforms)
  : core(core)
  , layouts(layouts)
  , uniforms(uniforms)
{
  const auto allocInfo = vk::DescriptorSetAllocateInfo()
    .setDescriptorPool(descriptorPool)
    .setDescriptorSetCount(layouts.size())
    .setPSetLayouts(layouts.data());

  ownedDescriptorSets = core.GetLogicalDevice().allocateDescriptorSetsUnique(allocInfo);
}

std::vector<vk::DescriptorSet> UniformsContext::GetUpdatedDescriptorSets()
{
  for (auto& [_, buf] : ownedBuffers)
    buf.Unmap();

  std::vector<vk::DescriptorSet> descriptorSets;
  descriptorSets.reserve(ownedDescriptorSets.size());

  for (const vk::UniqueDescriptorSet& s : ownedDescriptorSets)
    descriptorSets.push_back(s.get());

  std::vector<vk::WriteDescriptorSet> writesInfo;
  writesInfo.reserve(writes.size());

  for (const auto& [_, w] : writes)
    writesInfo.push_back(w);

  core.GetLogicalDevice().updateDescriptorSets(writesInfo.size(), writesInfo.data(), 0, nullptr);

  return descriptorSets;
}