#include "asset_storage.h"
#include <engine/components/static_mesh_component.h>
#include <engine/rendering/vulkan/core.h>

#include <iostream>
#include <array>
#include <map>
#include <tuple>
#include <stack>
#include <optional>

#include <ktx/ktx.h>
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

  enum class AttributeType
  {
    Index,
    Position,
    UV,
    Normal
  };

  inline std::string GetTypeString(AttributeType type)
  {
    switch (type)
    {
      case AttributeType::Index:
        return "__INDEX";
      case AttributeType::Position:
        return "POSITION";
      case AttributeType::UV:
        return "TEXCOORD_0";
      case AttributeType::Normal:
        return "NORMAL";
      default:
        return "UNKNOWN";
    }
  }
}

AssetStorage::AssetStorage(Core& vkCore)
  : vkCore(vkCore)
{
}

class AttributeAccessor
{
public:

  AttributeAccessor(const tinygltf::Model& model, const tinygltf::Primitive& primitive, AttributeType type)
  {
    const int accessorIndex = type == AttributeType::Index 
                                   ? primitive.indices 
                                   : primitive.attributes.at(GetTypeString(type));

    const tinygltf::Accessor& accessor = model.accessors[accessorIndex];

    const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];

    componentType = accessor.componentType;

    buf = reinterpret_cast<const float*>(model.buffers[bufferView.buffer].data.data() + bufferView.byteOffset + accessor.byteOffset);

    stride = accessor.ByteStride(bufferView);
    if (stride == -1)
      throw std::runtime_error("failed to calculate stride.");

    byteOffset = accessor.byteOffset + bufferView.byteOffset;

    count = accessor.count;

    i = 0;
  }

  AttributeAccessor(int componentType, const float* buf, int stride, int byteOffset, int count, int i)
    : componentType(componentType)
    , buf(buf)
    , stride(stride)
    , byteOffset(byteOffset)
    , count(count)
    , i(i)
  {
  }

  inline int GetCount() const
  {
    return count;
  }

  AttributeAccessor operator++(int)
  {
    AttributeAccessor it{ componentType, buf, stride, byteOffset,count,i };
    ++i;

    return it;
  }

  operator glm::vec3() const
  {
    const int typedStride = stride / sizeof(float);
    return glm::vec3{ buf[i * typedStride + 0], buf[i * typedStride + 1], buf[i * typedStride + 2] };
  }

  operator glm::vec2() const
  {
    const int typedStride = stride / sizeof(float);
    return glm::vec2{ buf[i * typedStride + 0], buf[i * typedStride + 1]};
  }

  operator uint32_t() const
  {
    switch (componentType)
    {
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
    {
      return reinterpret_cast<const unsigned char*>(buf)[i];
    }

    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
    {
      return reinterpret_cast<const unsigned short*>(buf)[i];
    }

    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
    {
      return reinterpret_cast<const unsigned int*>(buf)[i];
    }

    default:
      throw std::runtime_error("unsupported index type.");
    }
  }

private:
  int componentType;
  const float* buf;
  int stride;
  int byteOffset;
  int count;
  int i;
};

Image* AssetStorage::LoadCubeMap(const std::string& file, const std::string& cubeMapName)
{
  ktxTexture* texture;
  KTX_error_code result;
  ktx_size_t offset;
  ktx_uint8_t* image;

  result = ktxTexture_CreateFromNamedFile(file.c_str(),
    KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT,
    &texture);

  if (texture->numFaces != 6)
    throw std::runtime_error("texture is not cube map.");

  std::array<vk::DeviceSize, 6> offsets;
  for (int i = 0; i < 6; ++i)
  {
    ktx_size_t offset;
    result = ktxTexture_GetImageOffset(texture, 0, 0, i, &offset);

    if (result)
      throw std::runtime_error("failed to get texture offset.");

    offsets[i] = offset;
  }

  ktx_uint8_t* src = texture->pData;
  ktx_size_t size = texture->dataSize;

  Image cubeMap = vkCore.AllocateCubeMap(vk::Format::eR8G8B8A8Unorm, texture->pData, texture->dataSize, texture->baseWidth, texture->baseHeight, offsets);

  textures.insert({ cubeMapName, std::move(cubeMap) });

  return &textures.at(cubeMapName);
}

void AssetStorage::LoadTexture(const std::string& file, const std::string& cubeMapName)
{
  ktxTexture* texture;

  const KTX_error_code result = ktxTexture_CreateFromNamedFile(file.c_str(),
    KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT,
    &texture);

  if (result)
    throw std::runtime_error("failed to load texture");

  textures.insert(
    {
      cubeMapName,
      vkCore.Allocate2DImage(texture->pData, texture->dataSize, vk::Format::eR8G8B8A8Unorm, vk::Extent2D{ texture->baseWidth, texture->baseHeight }, vk::ImageUsageFlagBits::eSampled)
    }
  );
}

StaticModel* AssetStorage::LoadModel(const std::string& file, const std::string& modelName)
{
  const std::string rootUri = GetFolderPath(file);

  tinygltf::Model model;
  tinygltf::TinyGLTF loader;
  std::string err;
  std::string warn;

  bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, file.c_str());

  if (!warn.empty()) {
    printf("Warn: %s\n", warn.c_str());
  }

  if (!err.empty()) {
    printf("Err: %s\n", err.c_str());
  }

  if (!ret) {
    throw std::runtime_error("Failed to parse glTF");
  }

  LoadAllTextures(model, rootUri);

  StaticModel staticModel = ProcessModel(model, rootUri);
  staticModels.insert({ modelName, std::move(staticModel) });

  return &staticModels.at(modelName);
}

void AssetStorage::LoadAllTextures(const tinygltf::Model& model, const std::string& rootUri)
{
  for (const tinygltf::Image& imgSource : model.images)
  {
    if (imgSource.component != 4 || imgSource.pixel_type != TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
      throw std::runtime_error("only ub rgba textures are supported.");

    const vk::Extent2D imgExtent{ static_cast<uint32_t>(imgSource.width) , static_cast<uint32_t>(imgSource.height) };
    const int pixelSize = imgSource.component;
    const vk::DeviceSize srcSize = pixelSize * imgSource.width * imgSource.height;
    const void* src = reinterpret_cast<const void*>(imgSource.image.data());

    Image img = vkCore.Allocate2DImage(src, srcSize, vk::Format::eR8G8B8A8Unorm, imgExtent, vk::ImageUsageFlagBits::eSampled);

    textures.insert({ rootUri + "/" + imgSource.uri, std::move(img) });
  }
}

StaticModel AssetStorage::ProcessModel(const tinygltf::Model& model, const std::string& rootUri)
{
  StaticModel staticModel;

  for (tinygltf::Mesh mesh : model.meshes)
  {
    std::vector<StaticMeshVertex> vertices;
    std::vector<uint32_t> indices;

    for (tinygltf::Primitive primitive : mesh.primitives)
    {
      if (primitive.mode != TINYGLTF_MODE_TRIANGLES)
        throw std::runtime_error("unsupported primitive type.");

      AttributeAccessor posAccessor(model, primitive, AttributeType::Position);
      AttributeAccessor normalAccessor(model, primitive, AttributeType::Normal);
      AttributeAccessor uvAccessor(model, primitive, AttributeType::UV);
      AttributeAccessor indexAccessor(model, primitive, AttributeType::Index);

      for (int i = 0; i < posAccessor.GetCount(); ++i)
      {
        vertices.push_back({
          posAccessor++,
          normalAccessor++,
          uvAccessor++,
          });
      }

      for (int i = 0; i < indexAccessor.GetCount(); ++i)
        indices.push_back(indexAccessor++);
    }

    Buffer vertexBuffer = vkCore.AllocateDeviceBuffer(vertices.data(), vertices.size() * sizeof(StaticMeshVertex), vk::BufferUsageFlagBits::eVertexBuffer);
    Buffer indexBuffer = vkCore.AllocateDeviceBuffer(indices.data(), indices.size() * sizeof(uint32_t), vk::BufferUsageFlagBits::eIndexBuffer);

    Material material;
    const tinygltf::Material& gltfMaterial = model.materials[0];

    const std::string colorTextureName = rootUri + "/" + model.images[gltfMaterial.pbrMetallicRoughness.baseColorTexture.index].uri;
    material.colorTexture = &textures.at(colorTextureName);

    const std::string metallicRoughnessTextureName = rootUri + "/" + model.images[gltfMaterial.pbrMetallicRoughness.metallicRoughnessTexture.index].uri;
    material.metallicRoughnessTexture = &textures.at(metallicRoughnessTextureName);

    const std::string normalTextureName = rootUri + "/" + model.images[gltfMaterial.normalTexture.index].uri;
    material.normalTexture = &textures.at(normalTextureName);

    staticModel.meshes.push_back(
      StaticMesh{
        std::move(vertexBuffer),
        std::move(indexBuffer),
        static_cast<uint32_t>(indices.size())
      }
    );
    staticModel.materials.push_back(material);
  }

  return std::move(staticModel);
}

void AssetStorage::LoadStaticMesh(void* vertexSrc, size_t vertexSrcSize, void* indexSrc, uint32_t indexSrcSize, uint32_t indexCount, const std::string& meshName)
{
  Buffer vertexBuffer = vkCore.AllocateDeviceBuffer(vertexSrc, vertexSrcSize, vk::BufferUsageFlagBits::eVertexBuffer);
  Buffer indexBuffer = vkCore.AllocateDeviceBuffer(indexSrc, indexSrcSize, vk::BufferUsageFlagBits::eIndexBuffer);

  staticMeshes.insert({
    meshName, 
    StaticMesh{
      std::move(vertexBuffer),
      std::move(indexBuffer),
      static_cast<uint32_t>(indexCount)
    }
  });
}