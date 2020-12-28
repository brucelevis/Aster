#include "asset_storage.h"
#include <engine/components/static_mesh_component.h>
#include <engine/rendering/vulkan/core.h>

#include <iostream>
#include <array>
#include <map>
#include <tuple>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobjloader/tiny_obj_loader.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>

namespace tinyobj
{
  bool operator<(const index_t& l, const index_t& r)
  {
    return  std::tie(l.normal_index, l.texcoord_index, l.vertex_index) < std::tie(r.normal_index, r.texcoord_index, r.vertex_index);
  }
}

AssetStorage::AssetStorage(Core& vkCore)
  : vkCore(vkCore)
{
  
}

void AssetStorage::LoadModel(const std::string& objFile, const std::string& textureFile, const std::string& modelName)
{
  auto [vertexBuffer, indexBuffer, indicesSize] = LoadMesh(objFile);
  Image texture = LoadTexture(textureFile);

  staticMeshes.insert({
    modelName,
    StaticMesh{
      std::move(vertexBuffer),
      std::move(indexBuffer),
      static_cast<uint32_t>(indicesSize),
      std::move(texture),
    },
  });
}

std::tuple<Buffer, Buffer, uint32_t> AssetStorage::LoadMesh(const std::string& objFile)
{
  tinyobj::ObjReader reader;

  tinyobj::ObjReaderConfig config;
  config.triangulate = true;

  if (!reader.ParseFromFile(objFile, config)) {
    if (!reader.Error().empty()) {
      std::printf("tinyobj: %s: error: %s\n", objFile.c_str(), reader.Error().c_str());
      throw std::runtime_error(reader.Error());
    }
  }

  if (!reader.Warning().empty()) {
    std::printf("tinyobj: %s: warning: %s\n", objFile.c_str(), reader.Warning().c_str());
  }

  const tinyobj::attrib_t& attrib = reader.GetAttrib();
  std::vector<StaticMesh::Vertex> vertices;
  std::vector<uint32_t> indices;

  std::map<tinyobj::index_t, uint32_t> tinyobjIndexToVertexIndexMap;

  for (const tinyobj::shape_t& shape : reader.GetShapes())
  {
    for (const tinyobj::index_t& index : shape.mesh.indices)
    {
      if (tinyobjIndexToVertexIndexMap.find(index) != tinyobjIndexToVertexIndexMap.end())
      {
        indices.push_back(tinyobjIndexToVertexIndexMap.at(index));
      }
      else
      {
        tinyobjIndexToVertexIndexMap.insert({
          index,
          static_cast<uint32_t>(vertices.size())
          });

        StaticMesh::Vertex vertex;

        vertex.position.x = attrib.vertices[3 * index.vertex_index + 0];
        vertex.position.y = attrib.vertices[3 * index.vertex_index + 1];
        vertex.position.z = attrib.vertices[3 * index.vertex_index + 2];

        if (index.normal_index != -1)
        {
          vertex.normal = glm::vec3{
            attrib.vertices[3 * index.normal_index + 0],
            attrib.vertices[3 * index.normal_index + 1],
            attrib.vertices[3 * index.normal_index + 2],
          };
        }
        else
        {
          vertex.normal = glm::vec3{ 0.0f, -1.0f, 0.0f };
        }

        if (index.texcoord_index != -1)
        {
          vertex.uv = glm::vec2{
            attrib.texcoords[2 * index.texcoord_index + 0],
            attrib.texcoords[2 * index.texcoord_index + 1],
          };
        }
        else
        {
          vertex.uv = glm::vec2{ 0.0f, 0.0f };
        }

        indices.push_back(vertices.size());
        vertices.push_back(vertex);
      }
    }
  }

  Buffer vertexBuffer = vkCore.AllocateDeviceBuffer(vertices.data(), vertices.size() * sizeof(StaticMesh::Vertex), vk::BufferUsageFlagBits::eVertexBuffer);
  Buffer indexBuffer = vkCore.AllocateDeviceBuffer(indices.data(), indices.size() * sizeof(uint32_t), vk::BufferUsageFlagBits::eIndexBuffer);

  return { std::move(vertexBuffer), std::move(indexBuffer), static_cast<uint32_t>(indices.size()) };
}

Image AssetStorage::LoadTexture(const std::string& textureFile)
{
  int x, y, n;
  int channels_in_file;
  int force_rgba = 4;


  stbi_uc* uc = stbi_load(textureFile.c_str(), &x, &y, &channels_in_file, force_rgba);

  struct stbiDeleter
  {
    inline void operator()(void* img)
    {
      stbi_image_free(img);
    }
  };

  std::unique_ptr<void, stbiDeleter> src(
    reinterpret_cast<void*>(uc)
  );

  Image img = vkCore.Allocate2DImage(src.get(), x* y* (1 * 4), vk::Format::eR8G8B8A8Unorm, vk::Extent2D{ static_cast<uint32_t>(x),static_cast<uint32_t>(y) }, vk::ImageUsageFlagBits::eSampled);
  return std::move(img);
}