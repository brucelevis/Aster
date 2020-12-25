#pragma once

#include <engine/components/static_mesh_component.h>

#include <unordered_map>
#include <string>
#include <optional>

#define CUBE_STATIC_MESH std::string("__cube_mesh")

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

private:
  void InitializeBasicMeshes();

private:
  Core& vkCore;

  std::unordered_map<std::string, StaticMesh> staticMeshes;
};