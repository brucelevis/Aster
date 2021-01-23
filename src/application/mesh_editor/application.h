#pragma once

#include "renderer.h"
#include "scene.h"
#include "camera.h"

#include <memory>

namespace RHI::Vulkan
{
  class Core;
}

namespace Rendering
{
  class SceneRenderer;
}

struct GLFWwindow;

namespace App
{
  class Application
  {
  public:
    Application();

    void Start();

  private:
    GLFWwindow* m_Wnd;
    std::unique_ptr<RHI::Vulkan::Core> m_VkCore;
    std::unique_ptr<Rendering::SceneRenderer> m_SceneRenderer;

    Scene m_Scene;
    Camera m_Camera;
  };
}