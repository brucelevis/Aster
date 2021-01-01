#pragma once

#include <engine/components/static_mesh_component.h>

#include <unordered_map>
#include <string>
#include <optional>
#include <tuple>

class Core;
struct aiMesh;
struct aiMaterial;

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
  Core& vkCore;

  std::unordered_map<std::string, StaticModel> staticModels;
  std::unordered_map<std::string, Image> textures;
};