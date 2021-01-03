#pragma once

#include <ecs/BaseSystems.h>

#include <string>

namespace YAML
{
  class Node;
}

class Entity;
class Context;

class LevelInitializationSystem : public InitializationSystem
{
public:
  LevelInitializationSystem(Context* pContext, const YAML::Node& levelYaml);

  virtual void Initialize() override;

private:
  void LoadMeshes(const YAML::Node& config);
  void LoadDefaultMeshes();
  void LoadCubeMaps(const YAML::Node& config);
  void CreateEntities(const YAML::Node& config);
  void AddComponentToEntity(Entity* entity, const YAML::Node& componentDescription);
  void AddStaticMeshComponentToEntity(Entity* entity, const YAML::Node& componentDescription);
  void AddRootComponentToEntity(Entity* entity, const YAML::Node& componentDescription);
  void AddCameraComponentToEntity(Entity* entity, const YAML::Node& componentDescription);
  void AddSkyBoxComponentToEntity(Entity* entity, const YAML::Node& componentDescription);

private:
  const YAML::Node& levelYaml;
};
