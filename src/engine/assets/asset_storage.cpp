#include "asset_storage.h"
#include <engine/components/static_mesh_component.h>
#include <engine/rendering/vulkan/core.h>

#include <iostream>
#include <array>
#include <map>
#include <tuple>
#include <stack>
#include <optional>

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
// #define TINYGLTF_NOEXCEPTION // optional. disable exception handling.
#include <tinygltf/tiny_gltf.h>

namespace
{
  inline std::string GetFolderPath(const std::string& filePath)
  {
    return filePath.substr(0, filePath.find_last_of("\\/"));
  }
}

AssetStorage::AssetStorage(Core& vkCore)
  : vkCore(vkCore)
{

}

StaticModel* AssetStorage::LoadModel(const std::string& file, const std::string& modelName)
{
  StaticModel staticModel;
  const std::string fileFolder = GetFolderPath(file);

  tinygltf::Model model;
  tinygltf::TinyGLTF loader;
  std::string err;
  std::string warn;

  bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, file.c_str());
  //bool ret = loader.LoadBinaryFromFile(&model, &err, &warn, argv[1]); // for binary glTF(.glb)

  if (!warn.empty()) {
    printf("Warn: %s\n", warn.c_str());
  }

  if (!err.empty()) {
    printf("Err: %s\n", err.c_str());
  }

  if (!ret) {
    throw std::runtime_error("Failed to parse glTF");
  }

  for (const tinygltf::Image& imgSource : model.images)
  {
    if (imgSource.component != 4 || imgSource.pixel_type != TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
      throw std::runtime_error("only ub rgba textures are supported.");

    const vk::Extent2D imgExtent{ static_cast<uint32_t>(imgSource.width) , static_cast<uint32_t>(imgSource.height) };
    const int pixelSize = imgSource.component;
    const vk::DeviceSize srcSize = pixelSize * imgSource.width * imgSource.height;
    const void* src = reinterpret_cast<const void*>(imgSource.image.data());

    Image img = vkCore.Allocate2DImage(src, srcSize, vk::Format::eR8G8B8A8Unorm, imgExtent, vk::ImageUsageFlagBits::eSampled);

    textures.insert({ fileFolder + "/" + imgSource.uri, std::move(img) });
  }

  for (tinygltf::Mesh mesh : model.meshes)
  {
    std::vector<StaticMesh::Vertex> vertices;
    std::vector<uint32_t> indices;

    for (tinygltf::Primitive primitive : mesh.primitives)
    {
      if (primitive.mode != TINYGLTF_MODE_TRIANGLES)
        throw std::runtime_error("unsupported primitive type.");

      const tinygltf::Accessor& posAccessor = model.accessors[primitive.attributes.at("POSITION")];
      const tinygltf::BufferView& posBufferView = model.bufferViews[posAccessor.bufferView];
      const float* posBuffer = reinterpret_cast<float*>(model.buffers[posBufferView.buffer].data.data() + posBufferView.byteOffset + posAccessor.byteOffset);
      const int posStride = posAccessor.ByteStride(posBufferView) / sizeof(float);

      const tinygltf::Accessor& normalAccessor = model.accessors[primitive.attributes.at("NORMAL")];
      const tinygltf::BufferView& normalBufferView = model.bufferViews[normalAccessor.bufferView];
      const float* normalBuffer = reinterpret_cast<float*>(model.buffers[normalBufferView.buffer].data.data() + normalBufferView.byteOffset + normalAccessor.byteOffset);
      const int normalStride = normalAccessor.ByteStride(normalBufferView) / sizeof(float);

      const tinygltf::Accessor& uvAccessor = model.accessors[primitive.attributes.at("TEXCOORD_0")];
      const tinygltf::BufferView& uvBufferView = model.bufferViews[uvAccessor.bufferView];
      const float* uvBuffer = reinterpret_cast<float*>(model.buffers[uvBufferView.buffer].data.data() + uvBufferView.byteOffset + uvAccessor.byteOffset);
      const int uvStride = uvAccessor.ByteStride(uvBufferView) / sizeof(float);

      const tinygltf::Accessor& indexAccessor = model.accessors[primitive.indices];
      const tinygltf::BufferView& indexBufferView = model.bufferViews[indexAccessor.bufferView];
      const void* indexBuffer = reinterpret_cast<void*>(model.buffers[indexBufferView.buffer].data.data() + indexBufferView.byteOffset + indexAccessor.byteOffset);
      const int indexStride = indexAccessor.ByteStride(indexBufferView);

      if (posStride <= 0 || normalStride <= 0 || uvStride <= 0 || indexStride <= 0)
        throw std::runtime_error("failed to calculate data stride.");

      for (int i = 0; i < posAccessor.count; ++i)
      {
        vertices.push_back({
          glm::vec3{ posBuffer[i * posStride + 0], posBuffer[i * posStride + 1], posBuffer[i * posStride + 2]},
          glm::vec3{ normalBuffer[i * normalStride + 0], normalBuffer[i * normalStride + 1], normalBuffer[i * normalStride + 2]},
          glm::vec2{ uvBuffer[i * uvStride + 0], uvBuffer[i * uvStride + 1]},
        });
      }

      for (int i = 0; i < indexAccessor.count; ++i)
      {
        uint32_t index = 0;
        switch (indexAccessor.componentType)
        {
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
        {
          const unsigned char* buf = reinterpret_cast<const unsigned char*>(indexBuffer);
          index = buf[i];
          break;
        }

        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
        {
          const unsigned short* buf = reinterpret_cast<const unsigned short*>(indexBuffer);
          index = buf[i];
          break;
        }

        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
        {
          const unsigned int* buf = reinterpret_cast<const unsigned int*>(indexBuffer);
          index = buf[i];
          break;
        }

        default:
          throw std::runtime_error("unsupported index type.");
        }

        indices.push_back(index);
      }
    }

    Buffer vertexBuffer = vkCore.AllocateDeviceBuffer(vertices.data(), vertices.size() * sizeof(StaticMesh::Vertex), vk::BufferUsageFlagBits::eVertexBuffer);
    Buffer indexBuffer = vkCore.AllocateDeviceBuffer(indices.data(), indices.size() * sizeof(uint32_t), vk::BufferUsageFlagBits::eIndexBuffer);

    staticModel.meshes.push_back(
      StaticMesh{
        std::move(vertexBuffer),
        std::move(indexBuffer),
        static_cast<uint32_t>(indices.size())
      }
    );

    Material material;
    tinygltf::Material& gltfMaterial = model.materials[0];

    const std::string colorTextureName = fileFolder + "/" + model.images[gltfMaterial.pbrMetallicRoughness.baseColorTexture.index].uri;
    material.colorTexture = &textures.at(colorTextureName);

    const std::string metallicRoughnessTextureName = fileFolder + "/" + model.images[gltfMaterial.pbrMetallicRoughness.metallicRoughnessTexture.index].uri;
    material.metallicRoughnessTexture = &textures.at(metallicRoughnessTextureName);

    const std::string normalTextureName = fileFolder + "/" + model.images[gltfMaterial.normalTexture.index].uri;
    material.normalTexture = &textures.at(normalTextureName);

    staticModel.materials.push_back(material);
  }

  staticModels.insert({
    modelName,
    std::move(staticModel)
  });

  return &staticModels.at(modelName);
}
