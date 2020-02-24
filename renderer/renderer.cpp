#include "renderer.h"
#include <exception>
#include <stdexcept>

Renderer::Renderer()
    : window(nullptr)
{
}

Renderer::~Renderer()
{
    glfwTerminate();
}

void Renderer::initialize(const Config& config)
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(config.window.width, config.window.height, "Aster", NULL, NULL);
    if (window == NULL)
        throw std::runtime_error("Failed to create GLFW window");
    
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        throw std::runtime_error("Failed to initialize GLAD");
}

bool Renderer::shouldClose()
{
    return glfwWindowShouldClose(window);
}

void Renderer::drawFrame()
{

}