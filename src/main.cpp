#include <rendering/vulkan/core.h>
#include <rendering/vulkan/fileutils.h>
#include <rendering/vulkan/Shader.h>
#include <rendering/vulkan/pipeline.h>
#include <rendering/vulkan/vertexinputdeclaration.h>
#include <rendering/vulkan/FramebufferStorage.h>

#include <Windows.h>

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <glm/glm.hpp>
#include <ctime>

struct Vertex
{
  glm::vec3 position;
  glm::vec4 color;

  static VertexInputDeclaration GetVID()
  {
    VertexInputDeclaration vid;
    vid.AddBindingDescription(0, sizeof(Vertex));
    vid.AddAttributeDescription(vk::Format::eR32G32Sfloat, 0, 0, offsetof(Vertex, position));
    vid.AddAttributeDescription(vk::Format::eR32G32B32A32Sfloat, 0, 1, offsetof(Vertex, color));

    return vid;
  }
};

struct Resource
{
  float t;
};

#include <iostream>

int main()
{
  glfwInit();

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  GLFWwindow* window = glfwCreateWindow(800, 600, "Vulkan window", nullptr, nullptr);

  uint32_t count;
  const char** extensions = glfwGetRequiredInstanceExtensions(&count);

  Core core(window, extensions, count, vk::Extent2D{ 800, 600 });

  //create shaders
  Shader vertexShader = core.CreateShader("super_vertex", ReadFile("../data/shaders/spirv/dummy.vert.spv"));
  Shader fragmentShader = core.CreateShader("super_fragment", ReadFile("../data/shaders/spirv/dummy.frag.spv"));
  auto program = ShaderProgram(core, std::move(vertexShader), std::move(fragmentShader));

  //create buffer
  Vertex vertices[6]{
    { {0.0f, -0.5f, 0.0f}, {1.0f, 0.0f, 1.0f, 1.0f} },
    { {0.5f, 0.5f, 0.0f}, {1.0f, 0.0f, 1.0f, 1.0f}},
    { {-0.5f, 0.5f, 0.0f}, {1.0f, 0.0f, 1.0f, 1.0f}},

    { {0.0f, -0.7f, -1.0f}, {0.0f, 0.0f, 0.0f, 1.0f} },
    { {0.7f, 0.7, -1.0f}, {0.0f, 0.0f, 0.0f, 1.0f}},
    { {-0.2f, 0.9f, -1.0f}, {0.0f, 0.0f, 0.0f, 1.0f}}
  };

  Buffer vertBuffer = core.AllocateBuffer(sizeof(Vertex) * 6, vk::BufferUsageFlagBits::eVertexBuffer);
  vertBuffer.UploadMemory(vertices, sizeof(Vertex) * 6, 0);
  vk::Buffer vkbuf = vertBuffer.GetBuffer();
  vk::DeviceSize offset = 0;

  clock_t t1 = clock();

  while (!glfwWindowShouldClose(window)) {
    clock_t t2 = clock();
    float dt = (float)(t2 - t1) / (float)CLOCKS_PER_SEC;

    glfwPollEvents();

    RenderGraph* rg = core.BeginFrame();
    rg->AddRenderSubpass()
      .AddOutputColorAttachment(OutputColorAttachmentDescription{BACKBUFFER_RESOURCE_ID})
      .AddDepthStencilAttachment(DepthStencilAttachmentDescription{"depth"})
      .SetRenderCallback([&](FrameContext& context) 
      {
        const VertexInputDeclaration vid = Vertex::GetVID();
        const auto depthStencil = DepthStencilSettings()
          .SetDepthTestEnabled(true)
          .SetDepthWriteEnabled(true);
        
        Pipeline* p = context.pipelineStorage->GetPipeline(program, vid, vk::PrimitiveTopology::eTriangleList, depthStencil, context);

        UniformsAccessor* uniforms = context.uniformsAccessorStorage->GetUniformsAccessor(program);

        Resource* resourceUBO = uniforms->GetUniformBuffer<Resource>("Resource");
        resourceUBO->t = dt;
        std::vector<vk::DescriptorSet> descriptorSets = uniforms->GetUpdatedDescriptorSets();

        context.commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, p->GetPipeline());
        context.commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, p->GetLayout(), 0, descriptorSets.size(), descriptorSets.data(), 0, nullptr);
        context.commandBuffer.bindVertexBuffers(0, 1, &vkbuf, &offset);
        context.commandBuffer.draw(6, 1, 0, 0);
      });

    core.EndFrame();
  }

  glfwDestroyWindow(window);

  glfwTerminate();

}