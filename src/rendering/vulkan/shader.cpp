#include "Shader.h"

#include <iostream>

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

ShaderProgram::ShaderProgram(Shader&& v, Shader&& fr)
  : vertex(std::move(v))
  , fragment(std::move(fr))
{
  uniforms = vertex.GetUniformsDescriptions() + fragment.GetUniformsDescriptions();
}