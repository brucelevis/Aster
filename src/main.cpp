#include <engine/engine.h>
#include <engine/assets/asset_storage.h>
#include <engine/components/camera_component.h>
#include <engine/components/root_component.h>

#include <yaml-cpp/yaml.h>

namespace
{
  Entity* CreateCameraEntity(EntityManager* em)
  {
    Entity* e = em->NewEntity();

    RootComponent* root = e->AddComponent<RootComponent>("Root Component");
    root->transform.LocalPosition = { 0,0,0 };
    root->transform.LocalRotation = { 1.0f, 0.0f, 0.0f, 0.0f };

    CameraComponent* camera = e->AddComponent<CameraComponent>("Camera Component");
    camera->transform.LocalRotation = { 1.0f, 0.0f, 0.0f, 0.0f };
    camera->transform.AttachTo(&root->transform);
    camera->angle = glm::radians(45.0f);
    camera->width = 800;
    camera->height = 600;
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

class LevelInitializationSystem : public InitializationSystem
{
public:
  LevelInitializationSystem(Context* pContext, const std::string& levelYaml)
    : InitializationSystem(pContext)
    , levelYaml(levelYaml)
  {
  }

  virtual void Initialize() override
  {
    const YAML::Node config = YAML::LoadFile(levelYaml);

    LoadMeshes(config);
    CreateObjects(config);
  }

private:
  void LoadMeshes(const YAML::Node& config)
  {
    AssetStorage* assetStorage = reinterpret_cast<Engine*>(pContext->GetUserData())->GetAssetStorage();
    const YAML::Node meshes = config["meshes"];

    for (int i = 0; i < meshes.size(); ++i)
    {
      const YAML::Node mesh = meshes[i];
      const std::string name = mesh["name"].as<std::string>();
      const std::string meshFile = mesh["mesh_file"].as<std::string>();

      assetStorage->LoadModel(meshFile, "", name);
    }
  }

  void CreateObjects(const YAML::Node& config)
  {
    EntityManager* em = pContext->GetEntityManager();
    AssetStorage* as = reinterpret_cast<Engine*>(pContext->GetUserData())->GetAssetStorage();

    CreateCameraEntity(em);

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

private:
  std::string levelYaml;
};

class RotatorSystem : public LogicSystem
{
public:
  RotatorSystem(Context* pcontext)
    : LogicSystem(pcontext)
  {
    staticMeshGroup = pcontext->GetGroup<StaticMeshComponent>();
  }

  virtual void Update(const double dt) override
  {
    for (Entity* e : staticMeshGroup->GetEntities())
    {
      RootComponent* root = e->GetFirstComponent<RootComponent>();
      root->transform.LocalRotation = glm::angleAxis(glm::radians(float(dt * 20)), glm::vec3{ 1.0f, 1.0f, 0.0f }) * root->transform.LocalRotation;
    }
  }

private:
  Group* staticMeshGroup;
};

int main()
{
  Engine engine;
  engine.AddSystems([](Context& ecsContext)
  {
      ecsContext.AddInitializationSystems({
          new LevelInitializationSystem(&ecsContext, "../data/levels/cubes.yaml")
        }
      );

      ecsContext.AddLogicSystems({
        new RotatorSystem(&ecsContext)
      });
  });

  engine.Start();
}