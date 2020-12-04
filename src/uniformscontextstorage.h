#pragma once

#include "uniformscontext.h"

#include <vulkan/vulkan.hpp>
#include <map>

class Core;

class UniformsContextStorage
{
public:
  UniformsContextStorage(Core& core, vk::DescriptorPool descriptorPool);

  UniformsContext* GetUniformsContext(const std::vector<vk::DescriptorSetLayout>& layouts, const PipelineUniforms& uniforms);

  void Reset();

private:
  Core& core;
  vk::DescriptorPool descriptorPool;

  std::map<PipelineUniforms, std::unique_ptr<UniformsContext>> contexts;
};