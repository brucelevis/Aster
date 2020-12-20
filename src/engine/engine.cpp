#include "engine.h"

#include <engine/rendering/renderer.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <ctime>

Engine::Engine()
{
  glfwInit();

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  this->wnd = glfwCreateWindow(800, 600, "Vulkan window", nullptr, nullptr);

  uint32_t count;
  const char** extensions = glfwGetRequiredInstanceExtensions(&count);

  ecsContext.AddLogicSystems({
    new RenderSystem{&ecsContext,wnd, vk::Extent2D{800, 600}, extensions, count}
  });
}

Engine::~Engine()
{
  glfwDestroyWindow(wnd);
  glfwTerminate();
}

void Engine::Start()
{
  clock_t t1 = clock();
  while (!glfwWindowShouldClose(wnd)) {
    clock_t t2 = clock();
    float dt = static_cast<float>(t2 - t1) / static_cast<float>(CLOCKS_PER_SEC);

    glfwPollEvents();
    ecsContext.UpdateSystems(dt);
  }
}