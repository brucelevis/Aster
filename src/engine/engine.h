#pragma once

#include <ecs/Context.h>

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
};