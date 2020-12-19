#include "uniformsaccessorstorage.h"
#include "core.h"
#include "Shader.h"

UniformsAccessorStorage::UniformsAccessorStorage(Core& core, vk::DescriptorPool descriptorPool)
  : core(core)
  , descriptorPool(descriptorPool)
{
}

UniformsAccessor* UniformsAccessorStorage::GetUniformsAccessor(const ShaderProgram& program)
{
  const PipelineUniforms& uniforms = program.GetCombinedUniformsInformation();
  const std::vector<vk::DescriptorSetLayout>& layouts = program.GetLayouts();

  const auto it = contexts.find(uniforms);
  if (it != contexts.end())
  {
    return it->second.get();
  }

  auto uContext = std::make_unique<UniformsAccessor>(core, descriptorPool, layouts, uniforms);
  UniformsAccessor* context = uContext.get();

  contexts[uniforms] = std::move(uContext);

  return context;
}

void UniformsAccessorStorage::Reset()
{
  contexts.clear();
}