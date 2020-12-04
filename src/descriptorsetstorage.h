#pragma once

#include "Shader.h"

#include <map>
#include <vulkan/vulkan.hpp>

class Core;

typedef std::vector<UniformBindingDescription> UniformSetKey;

class DescriptorSetStorage
{
public:
  DescriptorSetStorage(Core& core);

  vk::DescriptorSetLayout GetDescriptorSetLayout(const UniformSetKey& key);

  std::vector<vk::DescriptorSetLayout> GetDescriptorSetLayouts(const PipelineUniforms& uniforms);

  inline void Clear()
  {
    storage.clear();
  }

private:
  Core& core;
  std::map<UniformSetKey, vk::UniqueDescriptorSetLayout> storage;
};