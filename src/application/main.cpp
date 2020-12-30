#include "level.h"
#include "camera.h"

#include <engine/engine.h>

int main()
{
  Engine::Settings settings;
  settings.window.isFullscreen = false;
  settings.window.width = 1920;
  settings.window.height = 1024;

  Engine engine{ settings };

  engine.AddSystems([](Context& ecsContext)
  {
      ecsContext.AddInitializationSystems({
          new LevelInitializationSystem(&ecsContext, "../data/levels/cubes.yaml")
        }
      );

      ecsContext.AddLogicSystems({
        new CameraMovementSystem(&ecsContext)
      });
  });

  engine.Start();
}