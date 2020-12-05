#pragma once

#include "uniformsaccessor.h"

#include <vulkan/vulkan.hpp>
#include <map>

class Core;
class ShaderProgram;

class UniformsAccessorStorage
{
public:
  UniformsAccessorStorage(Core& core, vk::DescriptorPool descriptorPool);

  UniformsAccessor* GetUniformsAccessor(const ShaderProgram& program);

  void Reset();

private:
  Core& core;
  vk::DescriptorPool descriptorPool;

  std::map<PipelineUniforms, std::unique_ptr<UniformsAccessor>> contexts;
};