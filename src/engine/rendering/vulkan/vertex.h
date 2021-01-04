#pragma once

#include "vertex_input_declaration.h"
#include <glm/glm.hpp>

struct StaticMeshVertex
{
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec2 uv;

  static inline VertexInputDeclaration GetVID()
  {
    VertexInputDeclaration vid;
    vid.AddBindingDescription(0, sizeof(StaticMeshVertex));
    vid.AddAttributeDescription(vk::Format::eR32G32B32Sfloat, 0, 0, offsetof(StaticMeshVertex, position));
    vid.AddAttributeDescription(vk::Format::eR32G32B32Sfloat, 0, 1, offsetof(StaticMeshVertex, normal));
    vid.AddAttributeDescription(vk::Format::eR32G32Sfloat, 0, 2, offsetof(StaticMeshVertex, uv));

    return vid;
  }
};

struct SkyBoxVertex
{
  glm::vec3 position;

  static inline VertexInputDeclaration GetVID()
  {
    VertexInputDeclaration vid;
    vid.AddBindingDescription(0, sizeof(SkyBoxVertex));
    vid.AddAttributeDescription(vk::Format::eR32G32B32Sfloat, 0, 0, offsetof(SkyBoxVertex, position));

    return vid;
  }
};

struct QuadVertex
{
  glm::vec2 position;

  static inline VertexInputDeclaration GetVID()
  {
    VertexInputDeclaration vid;
    vid.AddBindingDescription(0, sizeof(QuadVertex));
    vid.AddAttributeDescription(vk::Format::eR32G32Sfloat, 0, 0, offsetof(QuadVertex,position));

    return vid;
  }
};