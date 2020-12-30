#include "level.h"

#include <engine/assets/asset_storage.h>
#include <engine/components/camera_component.h>
#include <engine/components/root_component.h>
#include <engine/engine.h>

#include <ecs/Context.h>

#include <yaml-cpp/yaml.h>

namespace
{
  Entity* CreateCameraEntity(EntityManager* em, float width, float height)
  {
    Entity* e = em->NewEntity();

    RootComponent* root = e->AddComponent<RootComponent>("Root Component");
    root->transform.LocalPosition = { 0,0,0 };
    root->transform.LocalRotation = { 1.0f, 0.0f, 0.0f, 0.0f };

    CameraComponent* camera = e->AddComponent<CameraComponent>("Camera Component");
    camera->transform.LocalRotation = { 1.0f, 0.0f, 0.0f, 0.0f };
    camera->transform.AttachTo(&root->transform);
    camera->angle = glm::radians(45.0f);
    camera->width = width;
    camera->height = height;
    camera->zNear = 0.1f;
    camera->zFar = 100.0f;

    return e;
  }

  Entity* CreateObjectEntity(EntityManager* em, AssetStorage* as, const std::string& meshName, const glm::vec3& position, const glm::vec3& scale)
  {
    Entity* e = em->NewEntity();
    RootComponent* root = e->AddComponent<RootComponent>("Root Component");

    root->transform.LocalPosition = position;
    root->transform.LocalRotation = { 1.0f, 0.0f, 0.0f, 0.0f };
    root->transform.LocalScale = scale;

    StaticMeshComponent* staticMesh = e->AddComponent<StaticMeshComponent>("Static Mesh");
    staticMesh->mesh = as->GetStaticMesh(meshName);
    staticMesh->transform.AttachTo(&root->transform);
    staticMesh->transform.LocalRotation = { 1.0f, 0.0f, 0.0f, 0.0f };

    return e;
  }
}

LevelInitializationSystem::LevelInitializationSystem(Context* pContext, const YAML::Node& levelYaml)
  : InitializationSystem(pContext)
  , levelYaml(levelYaml)
{
}

void LevelInitializationSystem::Initialize()
{
  LoadMeshes(levelYaml);
  CreateObjects(levelYaml);
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

void LevelInitializationSystem::CreateObjects(const YAML::Node& config)
{
  EntityManager* em = pContext->GetEntityManager();
  AssetStorage* as = pContext->GetUserData<Engine*>()->GetAssetStorage();
  Engine::Settings engineSettings = pContext->GetUserData<Engine*>()->GetSettings();

  CreateCameraEntity(em, engineSettings.window.width, engineSettings.window.height);

  const YAML::Node objects = config["objects"];

  for (int i = 0; i < objects.size(); ++i)
  {
    const YAML::Node object = objects[i];
    const std::string meshName = object["mesh"].as<std::string>();
    const glm::vec3 position = {
      object["position"][0].as<float>(),
      object["position"][1].as<float>(),
      object["position"][2].as<float>(),
    };
    const glm::vec3 scale = {
      object["scale"][0].as<float>(),
      object["scale"][1].as<float>(),
      object["scale"][2].as<float>(),
    };

    CreateObjectEntity(em, as, meshName, position, scale);
  }
}