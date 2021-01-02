#include "Shader.h"
#include "core.h"

#include <iostream>

namespace
{
  vk::DescriptorType GetDescriptorType(UniformType type)
  {
    switch (type)
    {
    case UniformType::UniformBuffer:
      return vk::DescriptorType::eUniformBuffer;

    case UniformType::Sampler2D:
      return vk::DescriptorType::eCombinedImageSampler;

    case UniformType::SubpassInput:
      return vk::DescriptorType::eInputAttachment;

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

Shader::Shader(vk::Device logicalDevice, const std::string& name, const std::vector<uint32_t>& byteCode)
{
  uniforms = SpirvParser().ParseShader(byteCode);

  this->name = name;

  const auto shaderModuleCreateInfo = vk::ShaderModuleCreateInfo()
    .setCodeSize(byteCode.size() * sizeof(uint32_t))
    .setPCode(byteCode.data());

  shaderModule = logicalDevice.createShaderModuleUnique(shaderModuleCreateInfo);
}

vk::ShaderModule Shader::GetModule() const
{
  return shaderModule.get();
}

std::string Shader::GetName() const
{
  return name;
}

ShaderProgram::ShaderProgram(Core& core, Shader&& v, Shader&& fr)
  : core(core)
  , vertex(std::move(v))
  , fragment(std::move(fr))
{
  uniforms = vertex.GetUniformsDescriptions() + fragment.GetUniformsDescriptions();
  layouts = CreateLayouts(core, uniforms);
}

ShaderProgram::~ShaderProgram()
{
  for (const vk::DescriptorSetLayout& layout : layouts)
  {
    core.GetLogicalDevice().destroyDescriptorSetLayout(layout);
  }
}

std::vector<vk::DescriptorSetLayout> ShaderProgram::CreateLayouts(Core& core, const PipelineUniforms& uniforms) const
{
  std::vector<vk::DescriptorSetLayout> layouts;

  layouts.reserve(uniforms.sets.size());

  for (int i = 0; i < uniforms.sets.size(); ++i)
  {
    const UniformSetDescription& set = uniforms.sets[i];

    std::vector<vk::DescriptorSetLayoutBinding> bindings;
    for (int j = 0; j < set.bindings.size(); ++j)
    {
      const UniformBindingDescription& binding = set.bindings[j];

      if (binding.type == UniformType::None)
        continue;

      const auto bindingDescription = vk::DescriptorSetLayoutBinding()
        .setBinding(j)
        .setDescriptorCount(1)
        .setDescriptorType(GetDescriptorType(binding.type))
        .setStageFlags(GetShaderStageFlag(binding.stages));

      bindings.push_back(bindingDescription);
    }

    const auto layoutCreateInfo = vk::DescriptorSetLayoutCreateInfo()
      .setBindingCount(bindings.size())
      .setPBindings(bindings.data());
    //.setFlags()

    vk::DescriptorSetLayout layout = core.GetLogicalDevice().createDescriptorSetLayout(layoutCreateInfo);
    layouts.push_back(layout);
  }

  return layouts;
}