#pragma once

#include <engine/components/static_mesh_component.h>

#include <unordered_map>
#include <string>
#include <optional>
#include <tuple>

namespace RHI::Vulkan
{
  class Core;
}

namespace tinygltf
{
  class Model;
}

class AssetStorage
{
public:
  AssetStorage(RHI::Vulkan::Core& vkCore);

  inline RHI::Vulkan::StaticModel* GetStaticModel(const std::string name)
  {
    const auto it = staticModels.find(name);
    
    if (it != staticModels.end())
      return &it->second;

    return nullptr;
  }

  RHI::Vulkan::Image* LoadCubeMap(const std::string& file, const std::string& cubeMapName);

  void LoadTexture(const std::string& file, const std::string& textureName);

  inline RHI::Vulkan::Image* GetTexture(const std::string& cubeMapName)
  {
    const auto it = textures.find(cubeMapName);

    if (it != textures.end())
      return &it->second;

    return nullptr;
  }

  RHI::Vulkan::StaticModel* LoadModel(const std::string& file, const std::string& modelName);

  void LoadStaticMesh(void* vertexSrc, size_t vertexSrcSize, void* indexSrc, uint32_t indexSrcSize, uint32_t indexCount, const std::string& meshName);
  
  inline RHI::Vulkan::StaticMesh* GetStaticMesh(const std::string& meshName)
  {
    const auto it = staticMeshes.find(meshName);

    if (it != staticMeshes.end())
      return &it->second;

    return nullptr;
  }


private:
  void LoadAllTextures(const tinygltf::Model& model, const std::string& rootUri);
  RHI::Vulkan::StaticModel AssetStorage::ProcessModel(const tinygltf::Model& model, const std::string& rootUri);

private:
  RHI::Vulkan::Core& vkCore;

  std::unordered_map<std::string, RHI::Vulkan::StaticMesh> staticMeshes;
  std::unordered_map<std::string, RHI::Vulkan::StaticModel> staticModels;
  std::unordered_map<std::string, RHI::Vulkan::Image> textures;
  std::unordered_map<std::string, RHI::Vulkan::Image> cubeMaps;
};