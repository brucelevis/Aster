#pragma once

#include "vertex_input_declaration.h"
#include <glm/glm.hpp>

struct StaticMeshVertex
{
  alignas(16) glm::vec3 position;
  alignas(16) glm::vec3 color;

  static inline VertexInputDeclaration GetVID()
  {
    VertexInputDeclaration vid;
    vid.AddBindingDescription(0, sizeof(StaticMeshVertex));
    vid.AddAttributeDescription(vk::Format::eR32G32B32Sfloat, 0, 0, offsetof(StaticMeshVertex, position));
    vid.AddAttributeDescription(vk::Format::eR32G32B32Sfloat, 0, 1, offsetof(StaticMeshVertex, color));

    return vid;
  }
};