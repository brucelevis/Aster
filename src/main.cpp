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

  void CreateObjects(const YAML::Node& config)
  {
    EntityManager* em = pContext->GetEntityManager();
    AssetStorage* as = pContext->GetUserData<Engine*>()->GetAssetStorage();

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
      root->transform.LocalRotation = glm::angleAxis(glm::radians(float(dt * 20)), glm::vec3{ 0.0f, 1.0f, 0.0f }) * root->transform.LocalRotation;
    }
  }

private:
  Group* staticMeshGroup;
};

class CameraMovementSystem : public LogicSystem
{
public:
  CameraMovementSystem(Context* pContext)
    : LogicSystem(pContext)
  {
    cameraGroup = pContext->GetGroup<CameraComponent>();

    InputHandler* inputHandler = pContext->GetUserData<Engine*>()->GetInputHandler();
    inputHandler->DisableCursor();

    const float velocity = 5.0f;

    inputHandler->SetKeyBinding("W", velocity, [&](float value)
    {
        MoveForward(value);
    });

    inputHandler->SetKeyBinding("S", -velocity, [&](float value)
    {
      MoveForward(value);
    });

    inputHandler->SetKeyBinding("D", velocity, [&](float value)
    {
      MoveRight(value);
    });

    inputHandler->SetKeyBinding("A", -velocity, [&](float value)
    {
      MoveRight(value);
    });

    inputHandler->SetKeyBinding("SPACE", velocity, [&](float value)
    {
      MoveUp(value);
    });

    inputHandler->SetKeyBinding("C", -velocity, [&](float value)
    {
      MoveUp(value);
    });

    inputHandler->SetMouseInputBinding([&](float dx, float dy)
    {
      Rotate(dx,dy);
    });

    inputHandler->SetKeyBinding("ESCAPE", 0.0f, [](float) {
      exit(0);
    });
  }

  void MoveForward(float value)
  {
    movingForwardValue = value;
  }

  void MoveRight(float value)
  {
    movingRightValue = value;
  }

  void MoveUp(float value)
  {
    movingUpValue = value;
  }

  void Rotate(float dx, float dy)
  {
    CameraComponent* camera = cameraGroup->GetFirstNotNullEntity()->GetFirstComponent<CameraComponent>();

    if (dx > 0 || dx < 0)
    {
      angleX += dx * 0.1;

      if (angleX > 360.0f || angleX < -360.0f)
        angleX = 0.0f;
    }

    if (dy > 0 || dy < 0)
    {
      angleY += dy * 0.1;
      
      if (angleY > 90.0f)
        angleY = 90.0f;
      else if (angleY < -90.0f)
        angleY = -90.0f;
    }

    camera->transform.LocalRotation = glm::angleAxis(glm::radians(angleX), glm::vec3{ 0.0f, -1.0f, 0.0f }) 
                                    * glm::angleAxis(glm::radians(angleY), glm::vec3{ 1.0f, 0.0f, 0.0f });
  }

  virtual void Update(const double dt) override
  {
    pContext
      ->GetUserData<Engine*>()
      ->GetInputHandler()
      ->PollEvents();

    CameraComponent* camera = cameraGroup->GetFirstNotNullEntity()->GetFirstComponent<CameraComponent>();

    if (movingForwardValue > 0 || movingForwardValue < 0)
      camera->transform.LocalPosition += (movingForwardValue * (float)dt) * camera->transform.GetForwardVector();

    if (movingRightValue > 0 || movingRightValue < 0)
      camera->transform.LocalPosition += (movingRightValue * (float)dt) * camera->transform.GetRightVector();

    if (movingUpValue > 0 || movingUpValue < 0)
      camera->transform.LocalPosition += (movingUpValue * (float)dt) * glm::vec3{ 0.0f, -1.0f, 0.0f };
  }

private:
  Group* cameraGroup;
  float movingForwardValue = 0.0f;
  float movingRightValue = 0.0f;
  float movingUpValue = 0.0f;

  float angleX = 0.0f;
  float angleY = 0.0f;

};

int main()
{
  Engine::Settings settings;
  settings.window.isFullscreen = false;
  settings.window.width = 1920;
  settings.window.height = 1024;

  Engine engine{ settings };

  engine.AddSystems([](Context& ecsContext)
  {
      ecsContext.AddInitializationSystems({
          new LevelInitializationSystem(&ecsContext, "../data/levels/cubes.yaml")
        }
      );

      ecsContext.AddLogicSystems({
        new RotatorSystem(&ecsContext),
        new CameraMovementSystem(&ecsContext)
      });
  });

  engine.Start();
}