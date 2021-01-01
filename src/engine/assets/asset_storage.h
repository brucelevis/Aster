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

  StaticModel* LoadModel(const std::string& file, const std::string& modelName);

private:
  void LoadAllTextures(const tinygltf::Model& model, const std::string& rootUri);
  StaticModel AssetStorage::ProcessModel(const tinygltf::Model& model, const std::string& rootUri);

private:
  Core& vkCore;

  std::unordered_map<std::string, StaticModel> staticModels;
  std::unordered_map<std::string, Image> textures;
};