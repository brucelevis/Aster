#include "uniformscontextstorage.h"
#include "core.h"

UniformsContextStorage::UniformsContextStorage(Core& core, vk::DescriptorPool descriptorPool)
  : core(core)
  , descriptorPool(descriptorPool)
{
}

UniformsContext* UniformsContextStorage::GetUniformsContext(const std::vector<vk::DescriptorSetLayout>& layouts, const PipelineUniforms& uniforms)
{
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