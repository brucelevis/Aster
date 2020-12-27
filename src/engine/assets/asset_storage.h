#pragma once

#include <engine/components/static_mesh_component.h>

#include <unordered_map>
#include <string>
#include <optional>

class Core;

class AssetStorage
{
public:
  AssetStorage(Core& vkCore);

  inline StaticMesh* GetStaticMesh(const std::string name)
  {
    const auto it = staticMeshes.find(name);
    
    if (it != staticMeshes.end())
      return &it->second;

    return nullptr;
  }

  void LoadModel(const std::string& objFile, const std::string& textureFile, const std::string& modelName);

private:
  Core& vkCore;

  std::unordered_map<std::string, StaticMesh> staticMeshes;
};