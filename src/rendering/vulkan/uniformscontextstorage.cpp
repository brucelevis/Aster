#include "uniformscontextstorage.h"
#include "core.h"
#include "Shader.h"

UniformsContextStorage::UniformsContextStorage(Core& core, vk::DescriptorPool descriptorPool)
  : core(core)
  , descriptorPool(descriptorPool)
{
}

UniformsContext* UniformsContextStorage::GetUniformsContext(const ShaderProgram& program)
{
  const PipelineUniforms& uniforms = program.GetCombinedUniformsInformation();
  const std::vector<vk::DescriptorSetLayout>& layouts = program.GetLayouts();

  const auto it = contexts.find(uniforms);
  if (it != contexts.end())
  {
    return it->second.get();
  }

  auto uContext = std::make_unique<UniformsContext>(core, descriptorPool, layouts, uniforms);
  UniformsContext* context = uContext.get();

  contexts[uniforms] = std::move(uContext);

  return context;
}

void UniformsContextStorage::Reset()
{
  contexts.clear();
}