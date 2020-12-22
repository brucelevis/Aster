#pragma once

#include <ecs/Context.h>

#include <memory>

class Core;
struct GLFWwindow;

class Engine
{
public:
  Engine();
  ~Engine();

  void Start();

private:

private:
  Context ecsContext;
  GLFWwindow* wnd;
  std::unique_ptr<Core> vkCore;
};