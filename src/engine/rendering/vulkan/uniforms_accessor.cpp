#include "uniforms_accessor.h"
#include "core.h"

UniformsAccessor::UniformsAccessor(Core& core, vk::DescriptorPool descriptorPool, const std::vector<vk::DescriptorSetLayout>& layouts, const PipelineUniforms& uniforms)
  : core(core)
  , descriptorPool(descriptorPool)
  , layouts(layouts)
  , uniforms(uniforms)
{
  ownedDescriptorSets.resize(layouts.size());
}

std::vector<vk::DescriptorSet> UniformsAccessor::GetUpdatedDescriptorSets()
{
  std::vector<vk::WriteDescriptorSet> writesInfo;
  writesInfo.reserve(writes.size());

  for (const auto& [_, w] : writes)
    writesInfo.push_back(w);

  core.GetLogicalDevice().updateDescriptorSets(writesInfo.size(), writesInfo.data(), 0, nullptr);

  return currentDescriptorSets;
}