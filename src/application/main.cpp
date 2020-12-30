#include "level.h"
#include "camera.h"

#include <engine/engine.h>

#include <yaml-cpp/yaml.h>

int main()
{
  const YAML::Node config = YAML::LoadFile("../data/levels/cubes.yaml");
  const YAML::Node engineConfig = config["engine"];
  const YAML::Node applicationConfig = config["application"];

  Engine::Settings settings;
  settings.window.isFullscreen = engineConfig["window"]["fullscreen"].as<bool>();
  settings.window.width = engineConfig["window"]["width"].as<uint32_t>();
  settings.window.height = engineConfig["window"]["height"].as<uint32_t>();

  Engine engine{ settings };

  engine.AddSystems([&](Context& ecsContext)
  {
      ecsContext.AddInitializationSystems({
          new LevelInitializationSystem(&ecsContext, applicationConfig)
        }
      );

      ecsContext.AddLogicSystems({
        new CameraMovementSystem(&ecsContext)
      });
  });

  engine.Start();
}
