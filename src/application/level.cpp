#include "level.h"

#include <engine/assets/asset_storage.h>
#include <engine/components/camera_component.h>
#include <engine/components/root_component.h>
#include <engine/engine.h>

#include <ecs/Context.h>

#include <yaml-cpp/yaml.h>

LevelInitializationSystem::LevelInitializationSystem(Context* pContext, const YAML::Node& levelYaml)
  : InitializationSystem(pContext)
  , levelYaml(levelYaml)
{
}

void LevelInitializationSystem::Initialize()
{
  LoadMeshes(levelYaml);
  CreateEntities(levelYaml);
}

void LevelInitializationSystem::LoadMeshes(const YAML::Node& config)
{
  AssetStorage* assetStorage = pContext->GetUserData<Engine*>()->GetAssetStorage();
  const YAML::Node meshes = config["meshes"];

  for (int i = 0; i < meshes.size(); ++i)
  {
    const YAML::Node mesh = meshes[i];
    const std::string name = mesh["name"].as<std::string>();
    const std::string meshFile = mesh["mesh_file"].as<std::string>();
    const std::string textureFile = mesh["texture_file"].as<std::string>();

    assetStorage->LoadModel(meshFile, textureFile, name);
  }
}

void LevelInitializationSystem::CreateEntities(const YAML::Node& config)
{
  EntityManager* em = pContext->GetEntityManager();
  AssetStorage* as = pContext->GetUserData<Engine*>()->GetAssetStorage();

  const YAML::Node entities = config["entities"];

  for (int i = 0; i < entities.size(); ++i)
  {
    const YAML::Node entityDescription = entities[i];
    const YAML::Node components = entityDescription["components"];

    Entity* entity = em->NewEntity();

    for (int j = 0; j < components.size(); ++j)
      AddComponentToEntity(entity, components[j]);
  }
}

void LevelInitializationSystem::AddComponentToEntity(Entity* entity, const YAML::Node& componentDescription)
{
  const std::string type = componentDescription["type"].as<std::string>();

  if (type == "camera")
    AddCameraComponentToEntity(entity, componentDescription);

  if (type == "root")
    AddRootComponentToEntity(entity, componentDescription);

  if (type == "static_mesh")
    AddStaticMeshComponentToEntity(entity, componentDescription);
}

void LevelInitializationSystem::AddStaticMeshComponentToEntity(Entity* entity, const YAML::Node& componentDescription)
{
  const std::string meshName = componentDescription["mesh"].as<std::string>();
  const glm::vec3 position = {
    componentDescription["position"][0].as<float>(),
    componentDescription["position"][1].as<float>(),
    componentDescription["position"][2].as<float>(),
  };
  const glm::vec3 scale = {
    componentDescription["scale"][0].as<float>(),
    componentDescription["scale"][1].as<float>(),
    componentDescription["scale"][2].as<float>(),
  };

  const bool isAttachedToRootComponent = componentDescription["attach_to_root"].as<bool>();

  AssetStorage* as = pContext->GetUserData<Engine*>()->GetAssetStorage();

  StaticMeshComponent* staticMesh = entity->AddComponent<StaticMeshComponent>("Static Mesh");
  staticMesh->mesh = as->GetStaticMesh(meshName);
  
  staticMesh->transform.LocalPosition = position;
  staticMesh->transform.LocalScale = scale;
  staticMesh->transform.LocalRotation = { 1.0f, 0.0f, 0.0f, 0.0f };

  if (isAttachedToRootComponent)
  {
    RootComponent* root = entity->GetFirstComponent<RootComponent>();
    staticMesh->transform.AttachTo(&root->transform);
  }
}

void LevelInitializationSystem::AddRootComponentToEntity(Entity* entity, const YAML::Node& componentDescription)
{
  const glm::vec3 position = {
    componentDescription["position"][0].as<float>(),
    componentDescription["position"][1].as<float>(),
    componentDescription["position"][2].as<float>(),
  };
  const glm::vec3 scale = {
    componentDescription["scale"][0].as<float>(),
    componentDescription["scale"][1].as<float>(),
    componentDescription["scale"][2].as<float>(),
  };

  RootComponent* root = entity->AddComponent<RootComponent>("Root Component");
  root->transform.LocalPosition = position;
  root->transform.LocalScale = scale;
  root->transform.LocalRotation = { 1.0f, 0.0f, 0.0f, 0.0f };
}

void LevelInitializationSystem::AddCameraComponentToEntity(Entity* entity, const YAML::Node& componentDescription)
{
  Engine::Settings engineSettings = pContext->GetUserData<Engine*>()->GetSettings();

  const glm::vec3 position = {
    componentDescription["position"][0].as<float>(),
    componentDescription["position"][1].as<float>(),
    componentDescription["position"][2].as<float>(),
  };
  const float fov = componentDescription["fov"].as<float>();
  const float zNear = componentDescription["zNear"].as<float>();
  const float zFar = componentDescription["zFar"].as<float>();

  CameraComponent* camera = entity->AddComponent<CameraComponent>("Camera Component");
  camera->transform.LocalPosition = position;
  camera->transform.LocalRotation = { 1.0f, 0.0f, 0.0f, 0.0f };
  camera->angle = glm::radians(fov);
  camera->width = engineSettings.window.width;
  camera->height = engineSettings.window.height;
  camera->zNear = zNear;
  camera->zFar = zFar;
}