#include "application.h"

#include <engine/rendering/vulkan/core.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

namespace App
{
  Application::Application()
  {
    const int wndHeight = 1080;
    const int wndWidth = 1024;

    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    m_Wnd = glfwCreateWindow(wndWidth, wndHeight, "Vulkan window", nullptr, nullptr);

    uint32_t count;
    const char** extensions = glfwGetRequiredInstanceExtensions(&count);

    m_VkCore = std::make_unique<RHI::Vulkan::Core>( m_Wnd, extensions, count, vk::Extent2D{ wndWidth, wndHeight } );

    m_SceneRenderer = std::make_unique<Rendering::SceneRenderer>(m_VkCore.get());

    m_Camera = Camera{ {0.0f, -5.0f, -5.0f}, {wndWidth, wndHeight} };
  }

  void Application::Start()
  {
    while (!glfwWindowShouldClose(m_Wnd))
    {
      glfwPollEvents();
      m_SceneRenderer->Render(m_Scene, m_Camera);
    }
  }
}