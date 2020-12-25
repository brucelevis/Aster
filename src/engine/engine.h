#pragma once

#include <ecs/Context.h>

#include <memory>

class Core;
class AssetStorage;
struct GLFWwindow;

class Engine
{
public:
  Engine();
  ~Engine();

  void Start();

  inline AssetStorage* GetAssetStorage() const
  {
    return assetStorage.get();
  }

  inline void AddSystems(std::function<void(Context&)> addSystems)
  {
    addSystems(ecsContext);
  }

private:

private:
  Context ecsContext;
  GLFWwindow* wnd;
  std::unique_ptr<Core> vkCore;
  std::unique_ptr<AssetStorage> assetStorage;
};