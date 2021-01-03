#pragma once

#include <engine/components/static_mesh_component.h>

#include <unordered_map>
#include <string>
#include <optional>
#include <tuple>

class Core;
namespace tinygltf
{
  class Model;
}

class AssetStorage
{
public:
  AssetStorage(Core& vkCore);

  inline StaticModel* GetStaticModel(const std::string name)
  {
    const auto it = staticModels.find(name);
    
    if (it != staticModels.end())
      return &it->second;

    return nullptr;
  }

  Image* LoadCubeMap(const std::string& file, const std::string& cubeMapName);
  inline Image* GetCubeMap(const std::string& cubeMapName)
  {
    const auto it = cubeMaps.find(cubeMapName);

    if (it != cubeMaps.end())
      return &it->second;

    return nullptr;
  }

  StaticModel* LoadModel(const std::string& file, const std::string& modelName);

  void LoadStaticMesh(void* vertexSrc, size_t vertexSrcSize, void* indexSrc, uint32_t indexSrcSize, uint32_t indexCount, const std::string& meshName);
  
  inline StaticMesh* GetStaticMesh(const std::string& meshName)
  {
    const auto it = staticMeshes.find(meshName);

    if (it != staticMeshes.end())
      return &it->second;

    return nullptr;
  }


private:
  void LoadAllTextures(const tinygltf::Model& model, const std::string& rootUri);
  StaticModel AssetStorage::ProcessModel(const tinygltf::Model& model, const std::string& rootUri);

private:
  Core& vkCore;

  std::unordered_map<std::string, StaticMesh> staticMeshes;
  std::unordered_map<std::string, StaticModel> staticModels;
  std::unordered_map<std::string, Image> textures;
  std::unordered_map<std::string, Image> cubeMaps;
};