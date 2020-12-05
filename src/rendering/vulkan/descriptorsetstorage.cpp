#include "descriptorsetstorage.h"
#include "core.h"

namespace
{
  vk::DescriptorType GetDescriptorType(UniformType type)
  {
    switch (type)
    {
    case UniformType::UniformBuffer:
      return vk::DescriptorType::eUniformBuffer;

    default:
      throw std::runtime_error("unknown uniform type.");
    }
  }

  vk::ShaderStageFlags GetShaderStageFlag(ShaderStages stages)
  {
    vk::ShaderStageFlags bits;

    if (HAS_STAGE(stages, SHADER_VERTEX_STAGE))
      bits |= vk::ShaderStageFlagBits::eVertex;

    if (HAS_STAGE(stages, SHADER_FRAGMENT_STAGE))
      bits |= vk::ShaderStageFlagBits::eFragment;

    return bits;
  }
}

DescriptorSetStorage::DescriptorSetStorage(Core& core)
  : core(core)
{
}

vk::DescriptorSetLayout DescriptorSetStorage::GetDescriptorSetLayout(const UniformSetKey& bindingsKey)
{
  auto it = storage.find(bindingsKey);

  if (it != storage.end())
    return it->second.get();

  std::vector<vk::DescriptorSetLayoutBinding> bindings;
  for (int i = 0; i < bindingsKey.size(); ++i)
  {
    if (bindingsKey[i].type == UniformType::None)
      continue;

    const auto binding = vk::DescriptorSetLayoutBinding()
      .setBinding(i)
      .setDescriptorCount(1)
      .setDescriptorType(GetDescriptorType(bindingsKey[i].type))
      .setStageFlags(GetShaderStageFlag(bindingsKey[i].stages));

    bindings.push_back(binding);
  }

  const auto createInfo = vk::DescriptorSetLayoutCreateInfo()
    .setBindingCount(bindings.size())
    .setPBindings(bindings.data());
    //.setFlags()

  vk::UniqueDescriptorSetLayout uniqueLayout = core.GetLogicalDevice().createDescriptorSetLayoutUnique(createInfo);
  vk::DescriptorSetLayout layout = uniqueLayout.get();

  storage[bindingsKey] = std::move(uniqueLayout);
  return layout;
}

std::vector<vk::DescriptorSetLayout> DescriptorSetStorage::GetDescriptorSetLayouts(const PipelineUniforms& uniforms)
{
  std::vector<vk::DescriptorSetLayout> layouts;

  for (const UniformSetDescription& setDescription : uniforms.sets)
  {
    vk::DescriptorSetLayout layout = GetDescriptorSetLayout(setDescription.bindings);
    layouts.push_back(layout);
  }

  return layouts;
}