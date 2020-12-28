#pragma once

#include <engine/components/transform.h>
#include <engine/rendering/vulkan/buffer.h>
#include <engine/rendering/vulkan/vertex.h>
#include <engine/rendering/vulkan/image.h>

#include <ecs/BaseComponent.h>

#include <glm/glm.hpp>
#include <vector>

struct StaticMesh
{
  Buffer vertices;
  Buffer indices;
  unsigned int indexCount;
  Image texture;

  typedef StaticMeshVertex Vertex;
};

struct StaticMeshComponent : public BaseComponent
{
  Transform transform;
  StaticMesh* mesh;
};
