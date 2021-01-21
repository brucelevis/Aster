#include <engine/rendering/renderer.h>
#include <engine/rendering/imgui_backend.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>


int main()
{
  const int wndHeight = 1080;
  const int wndWidth = 1024;

  glfwInit();

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  GLFWwindow* wnd = glfwCreateWindow(wndWidth, wndHeight, "Vulkan window", nullptr, nullptr);

  uint32_t count;
  const char** extensions = glfwGetRequiredInstanceExtensions(&count);

  RHI::Vulkan::Core vkCore{ wnd, extensions, count, vk::Extent2D{ wndWidth, wndHeight } };
  GUI::ImGuiBackend imguiBackend(wnd, vkCore);

  while (!glfwWindowShouldClose(wnd))
  {
    glfwPollEvents();

    RHI::Vulkan::RenderGraph* rg = vkCore.BeginFrame();

    imguiBackend.AddGUIRenderingSubpass(rg, false, []() {
      ImGui::Begin("test window");
      ImGui::Text("Test text, int:%d", 221);
      ImGui::Button("test label");
      ImGui::End();
    });

    vkCore.EndFrame();

  }
}
