#pragma once

#include "shaderparsing.h"

#include <vulkan/vulkan.hpp>

#include <string>

class Shader
{
public:
  Shader(vk::Device logicalDevice, const std::string& name, const std::vector<uint32_t>& byteCode);

  vk::ShaderModule GetModule() const;

  std::string GetName() const;

  inline const PipelineUniforms& GetUniformsDescriptions() const
  {
    return uniforms;
  }

private:
  std::string name;
  vk::UniqueShaderModule shaderModule;

  PipelineUniforms uniforms;
};

class ShaderProgram
{
public:
  ShaderProgram(Shader&& vertex, Shader&& fragment);

  inline const Shader& GetVertexShader() const
  {
    return vertex;
  }

  inline const Shader& GetFragmentShader() const
  {
    return fragment;
  }

  inline const PipelineUniforms& GetCombinedUniformsInformation() const
  {
    return uniforms;
  }

private:
  Shader vertex;
  Shader fragment;

  PipelineUniforms uniforms;
};