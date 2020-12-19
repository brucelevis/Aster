#pragma once

#include "shaderparsing.h"

#include <vulkan/vulkan.hpp>

#include <string>

class Core;

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
  ShaderProgram(Core& core, Shader&& vertex, Shader&& fragment);
  ~ShaderProgram();

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

  inline const std::vector<vk::DescriptorSetLayout> GetLayouts() const
  {
    return layouts;
  }

private:
  std::vector<vk::DescriptorSetLayout> CreateLayouts(Core& core, const PipelineUniforms& uniforms) const;

private:
  Core& core;

  Shader vertex;
  Shader fragment;

  PipelineUniforms uniforms;
  std::vector<vk::DescriptorSetLayout> layouts;
};