#pragma once

#include "transform.h"

#include <ecs/BaseComponent.h>

//in general, everything that has to be visible should attach to render component
class RootComponent : public BaseComponent
{
  Transform transform;
};