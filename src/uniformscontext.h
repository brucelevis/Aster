#pragma once

#include <vulkan/vulkan.hpp>
#include "Shader.h"
#include "buffer.h"

class Core;

class UniformsContext
{
public:
  UniformsContext(Core& core, vk::DescriptorPool descriptorPool, const std::vector<vk::DescriptorSetLayout>& layouts, const PipelineUniforms& uniforms);

  template<class T>
  T* GetUniformBuffer(const UniformName& name)
  {
    const UniformSetPair setBinding = uniforms.GetSetBindingPair(name);
    const UniformBindingDescription& bindingDescription = uniforms.GetBindingDescription(setBinding.set, setBinding.binding);
    
    if (bindingDescription.type != UniformType::UniformBuffer)
      throw std::runtime_error("UniformsContext::GetUniformBuffer, uniform is not UBO type.");

    if (sizeof(T) != bindingDescription.size)
      throw std::runtime_error("UniformsContext::GetUniformBuffer, uniform's size is not equal to the requested mapping structure.");

    const auto it = ownedBuffers.find(name);
    if (it != ownedBuffers.end())
      return reinterpret_cast<T*>(it->second.GetMappedMemory());

    Buffer tmpBuf = core.AllocateBuffer(bindingDescription.size, vk::BufferUsageFlagBits::eUniformBuffer);
    ownedBuffers.insert({ name, std::move(tmpBuf) });
    const Buffer& buf = ownedBuffers.at(name);

    vk::DescriptorSet descriptorSet = ownedDescriptorSets.at(setBinding.set).get();

    writes[setBinding] = vk::WriteDescriptorSet()
      .setDescriptorCount(1)
      .setDescriptorType(vk::DescriptorType::eUniformBuffer)
      .setDstArrayElement(0)
      .setDstBinding(setBinding.binding)
      .setDstSet(descriptorSet)
      .setPBufferInfo(&buf.GetFullBufferUpdateInfo());

    return reinterpret_cast<T*>(ownedBuffers.at(name).Map());
  }
  
  std::vector<vk::DescriptorSet> GetUpdatedDescriptorSets();

private:
  Core& core;
  const std::vector<vk::DescriptorSetLayout>& layouts;
  PipelineUniforms uniforms;

  std::vector<vk::UniqueDescriptorSet> ownedDescriptorSets;
  std::map<UniformName, Buffer> ownedBuffers;
  std::map<UniformSetPair, vk::WriteDescriptorSet> writes;
};