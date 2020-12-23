#pragma once

#include <vulkan/vulkan.hpp>
#include "Shader.h"
#include "buffer.h"

class Core;

class UniformsAccessor
{
public:
  UniformsAccessor(Core& core, vk::DescriptorPool descriptorPool, const std::vector<vk::DescriptorSetLayout>& layouts, const PipelineUniforms& uniforms);

  template<class T>
  void SetUniformBuffer(const UniformName& name, const T* data)
  {
    const UniformSetPair setBinding = uniforms.GetSetBindingPair(name);
    const UniformBindingDescription& bindingDescription = uniforms.GetBindingDescription(setBinding.set, setBinding.binding);

    if (bindingDescription.type != UniformType::UniformBuffer)
      throw std::runtime_error("UniformsAccessor::GetUniformBuffer, uniform is not UBO type.");

    if (sizeof(T) != bindingDescription.size)
      throw std::runtime_error("UniformsAccessor::GetUniformBuffer, uniform's size is not equal to the requested mapping structure.");

    HostBuffer buf = core.AllocateHostBuffer(bindingDescription.size, vk::BufferUsageFlagBits::eUniformBuffer);
    buf.UploadMemory(data, sizeof(T), 0);
    const vk::DescriptorBufferInfo& dscBufInfo = buf.GetFullBufferUpdateInfo();
    ownedBuffers.push_back(std::move(buf));

    const bool isUboForThisBindingAlreadySet = writes.find(setBinding) != writes.end();
    vk::DescriptorSet& dscSet = currentDescriptorSets[setBinding.set];

    if (dscSet == vk::DescriptorSet{} || isUboForThisBindingAlreadySet)
    {
      const auto allocInfo = vk::DescriptorSetAllocateInfo()
        .setDescriptorPool(descriptorPool)
        .setDescriptorSetCount(1)
        .setPSetLayouts(&layouts[setBinding.set]);

      vk::UniqueDescriptorSet s = std::move(core.GetLogicalDevice().allocateDescriptorSetsUnique(allocInfo)[0]);
      dscSet = s.get();

      ownedDescriptorSets.push_back(std::move(s));
    }

    writes[setBinding] = vk::WriteDescriptorSet()
      .setDescriptorCount(1)
      .setDescriptorType(vk::DescriptorType::eUniformBuffer)
      .setDstArrayElement(0)
      .setDstBinding(setBinding.binding)
      .setDstSet(dscSet)
      .setPBufferInfo(&buf.GetFullBufferUpdateInfo());
  }

  std::vector<vk::DescriptorSet> GetUpdatedDescriptorSets();

private:
  Core& core;
  vk::DescriptorPool descriptorPool;
  const std::vector<vk::DescriptorSetLayout>& layouts;
  PipelineUniforms uniforms;

  std::vector<vk::DescriptorSet> currentDescriptorSets;
  std::vector<vk::UniqueDescriptorSet> ownedDescriptorSets;
  std::vector<Buffer> ownedBuffers;

  std::map<UniformSetPair, vk::WriteDescriptorSet> writes;
};