#pragma once

#include <engine/rendering/imgui_backend.h>

struct GLFWwindow;

namespace RHI::Vulkan
{
  class Core;
}

class ApplicationBase
{
public:
  ApplicationBase(GLFWwindow* wnd, RHI::Vulkan::Core* vkCore, int wndWidth, int wndHeight);

  void Start();

protected:
  virtual void Dt(float dt){}

protected:
  GLFWwindow* m_Wnd;
  int m_WndWidth;
  int m_WndHeight;

  RHI::Vulkan::Core* m_VkCore;
  GUI::ImGuiBackend m_GUI;
};
