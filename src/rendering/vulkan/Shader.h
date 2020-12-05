#pragma once

#include <vector>

#include <vulkan/vulkan.hpp>

#include <map>
#include <string>
#include <tuple>

enum class UniformType
{
  None,
  UniformBuffer
};

typedef unsigned int ShaderStages;
#define SHADER_VERTEX_STAGE (ShaderStages) 0x1
#define SHADER_FRAGMENT_STAGE (ShaderStages) 0x2
#define HAS_STAGE(v, s) (v & s)

struct UniformBindingDescription
{
  UniformType type = UniformType::None;
  ShaderStages stages = 0;
  size_t size = 0;

  inline bool IsEqualWithoutStages(const UniformBindingDescription& r)
  {
    return std::tie(type, size) != std::tie(r.type, size);
  }

  inline bool operator<(const UniformBindingDescription& r) const
  {
    return std::tie(type, stages, size) < std::tie(r.type, r.stages, r.size);
  }
};

struct UniformSetDescription
{
  bool inUse = false;
  std::vector<UniformBindingDescription> bindings;

  inline bool operator< (const UniformSetDescription& r) const
  {
    return std::tie(inUse, bindings) < std::tie(r.inUse, r.bindings);
  }
};

typedef std::string UniformName;

struct UniformSetPair
{
  unsigned int set = 0;
  unsigned int binding = 0;

  inline bool operator< (const UniformSetPair& r) const
  {
    return  std::tie(set, binding) < std::tie(r.set, r.binding);
  }

  inline bool operator != (const UniformSetPair& r) const
  {
    return std::tie(set, binding) != std::tie(r.set, r.binding);
  }
};

struct PipelineUniforms
{
  std::vector<UniformSetDescription> sets;
  std::map<UniformName, UniformSetPair> uniformsMap;

  void AddUniform(unsigned int set, unsigned int binding, const std::string& name ,const UniformBindingDescription& description);

  const UniformBindingDescription& GetBindingDescription(const unsigned int set, const unsigned int binding) const;

  const UniformBindingDescription& GetBindingDescription(const UniformName& name) const;

  const UniformSetPair& GetSetBindingPair(const UniformName& name) const;

  inline bool operator<(const PipelineUniforms& r) const
  {
    return sets < r.sets;
  }
};

PipelineUniforms operator+(const PipelineUniforms& l, const PipelineUniforms& r);

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
  void ParseShader(const std::vector<uint32_t>& byteCode);

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