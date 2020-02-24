#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Renderer
{
public:
    struct Config 
    {
        struct
        {
            unsigned height, width;
        } window;
    };
public:
    Renderer();
    ~Renderer();
    void initialize(const Config& config);

    bool shouldClose();
    void drawFrame();

private:
    GLFWwindow* window;
};