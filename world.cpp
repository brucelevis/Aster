#include "world.h"
#include "renderer/renderer.h"
#include <ctime>

World::World()
{
    auto config = Renderer::Config();
    config.window.height = 600;
    config.window.width  = 800;
    renderer = new Renderer();
    renderer->initialize(config);
}

void World::run()
{
    clock_t t1;
    
    t1 = std::clock();
    while(false == renderer->shouldClose())
    {
        const float dt = static_cast<float>(std::clock() - t1) / CLOCKS_PER_SEC;
        step(dt);
    }
}

void World::step(const float dt)
{
    renderer->drawFrame();
}