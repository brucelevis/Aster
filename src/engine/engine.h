#pragma once

#include <engine/input/input_handler.h>

#include <ecs/Context.h>

#include <memory>

class AssetStorage;
class Core;
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

  inline InputHandler* GetInputHandler() const
  {
    return inputHandler.get();
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
  std::unique_ptr<InputHandler> inputHandler;
};