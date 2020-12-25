#include <engine/engine.h>
#include <engine/assets/asset_storage.h>
#include <engine/rendering/components/camera_component.h>
#include <engine/components/root_component.h>

class DefaultLevelInitializationSystem : public InitializationSystem
{
public:
  explicit DefaultLevelInitializationSystem(Context* pcontext)
    : InitializationSystem(pcontext)
  {
  }

  virtual void Initialize() override
  {
    EntityManager* em = pContext->GetEntityManager();
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


    Entity* boxEntity = em->NewEntity();
    RootComponent* boxRoot = boxEntity->AddComponent<RootComponent>("Root Component");
    boxRoot->transform.LocalPosition = { 2,2,-8 };
    boxRoot->transform.LocalRotation = { 1.0f, 0.0f, 0.0f, 0.0f };

    Engine* engine = reinterpret_cast<Engine*>(pContext->GetUserData());
    AssetStorage* assetStorage = engine->GetAssetStorage();
    StaticMeshComponent* staticMesh = boxEntity->AddComponent<StaticMeshComponent>("Static Mesh");
    staticMesh->mesh = assetStorage->GetStaticMesh(CUBE_STATIC_MESH);
    staticMesh->transform.AttachTo(&boxRoot->transform);
    staticMesh->transform.LocalRotation = { 1.0f, 0.0f, 0.0f, 0.0f };
  }

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
    Entity* e = staticMeshGroup->GetFirstNotNullEntity();
    RootComponent* root = e->GetFirstComponent<RootComponent>();
    root->transform.LocalRotation = glm::angleAxis(glm::radians(float(dt * 20)), glm::vec3{ 1.0f, 1.0f, 0.0f }) * root->transform.LocalRotation;
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
          new DefaultLevelInitializationSystem(&ecsContext)
        }
      );

      ecsContext.AddLogicSystems({
        new RotatorSystem(&ecsContext)
      });
  });

  engine.Start();
}