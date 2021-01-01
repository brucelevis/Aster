#include "asset_storage.h"
#include <engine/components/static_mesh_component.h>
#include <engine/rendering/vulkan/core.h>

#include <iostream>
#include <array>
#include <map>
#include <tuple>
#include <stack>
#include <optional>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>

namespace
{
  const std::string DefaultTextureFile = "../data/meshes/default/default.jpg";

  inline std::string GetFolderPath(const std::string& filePath)
  {
    return filePath.substr(0, filePath.find_last_of("\\/"));
  }

  inline glm::vec3 AiVec3dToGlm(const aiVector3D& vec)
  {
    return glm::vec3{ vec.x, vec.y, vec.z };
  }

  inline glm::vec2 AiVec3dToGlm2D(const aiVector3D& vec)
  {
    return glm::vec2{ vec.x, vec.y};
  }


  struct TextureFiles
  {
    std::optional<std::string> color;
    std::optional<std::string> metallic;
    std::optional<std::string> rougness;
    std::optional<std::string> normals;
  };

  std::tuple<std::vector<StaticMesh::Vertex>, std::vector<uint32_t>, TextureFiles> GatherMeshData(aiMesh* mesh, aiMaterial* material, const std::string& meshFileDir)
  {
    std::vector<StaticMesh::Vertex> vertices;
    vertices.reserve(mesh->mNumVertices);

    for (int nVertex = 0; nVertex < mesh->mNumVertices; ++nVertex)
    {
      vertices.push_back(StaticMesh::Vertex{
        AiVec3dToGlm(mesh->mVertices[nVertex]),
        AiVec3dToGlm(mesh->mNormals[nVertex]),
        AiVec3dToGlm2D(mesh->mTextureCoords[0][nVertex])
        });
    }

    std::vector<uint32_t> indices;
    indices.reserve(mesh->mNumFaces * 3);

    for (int nFace = 0; nFace < mesh->mNumFaces; ++nFace)
    {
      indices.push_back({
        mesh->mFaces[nFace].mIndices[0]
        });
      indices.push_back({
        mesh->mFaces[nFace].mIndices[1]
        });
      indices.push_back({
        mesh->mFaces[nFace].mIndices[2]
        });
    }

    TextureFiles textureFiles;

    auto getTexture = [&](aiTextureType type) -> std::optional<std::string>
    {
      if (material->GetTextureCount(type) != 0)
      {
        aiString path;
        if (material->GetTexture(type, 0, &path))
          return std::nullopt;

        return meshFileDir + "/" + std::string(path.C_Str(), path.length);
      }

      return std::nullopt;
    };

    textureFiles.color = getTexture(aiTextureType::aiTextureType_BASE_COLOR);
    // let's try non pbr texture
    if (textureFiles.color.has_value() == false)
      textureFiles.color = getTexture(aiTextureType::aiTextureType_DIFFUSE);

    textureFiles.metallic = getTexture(aiTextureType::aiTextureType_METALNESS);
    textureFiles.rougness = getTexture(aiTextureType::aiTextureType_DIFFUSE_ROUGHNESS);
    textureFiles.normals = getTexture(aiTextureType::aiTextureType_NORMALS);

    return { vertices, indices, textureFiles };
  }
}

AssetStorage::AssetStorage(Core& vkCore)
  : vkCore(vkCore)
{
  LoadTexture(DefaultTextureFile);
}

StaticModel* AssetStorage::LoadModel(const std::string& file, const std::string& modelName)
{
  StaticModel staticModel;

  Assimp::Importer importer;
  const aiScene* scene = importer.ReadFile(file, aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_FlipUVs);

  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    throw std::runtime_error(importer.GetErrorString());

  std::stack<aiNode*> aiNodesToProcess;
  aiNodesToProcess.push(scene->mRootNode);

  while (!aiNodesToProcess.empty())
  {
    aiNode* node = aiNodesToProcess.top();
    aiNodesToProcess.pop();

    for (int i = 0; i < node->mNumChildren; ++i)
      aiNodesToProcess.push(node->mChildren[i]);

    for (int i = 0; i < node->mNumMeshes; ++i)
    {
      aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
      aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];

      auto [staticMesh, meshMaterial] = ProcessMesh(mesh, mat, GetFolderPath(file));

      staticModel.meshes.push_back(std::move(staticMesh));
      staticModel.materials.push_back(meshMaterial);
    }
  }

  staticModels.insert({
    modelName,
    std::move(staticModel)
  });

  return &staticModels.at(modelName);
}

std::tuple<StaticMesh, Material> AssetStorage::ProcessMesh(aiMesh* mesh, aiMaterial* mat, const std::string& meshFileDir)
{
  auto [vertices, indices, textureFiles] = GatherMeshData(mesh, mat, meshFileDir);

  Buffer vertexBuffer = vkCore.AllocateDeviceBuffer(vertices.data(), vertices.size() * sizeof(StaticMesh::Vertex), vk::BufferUsageFlagBits::eVertexBuffer);
  Buffer indexBuffer = vkCore.AllocateDeviceBuffer(indices.data(), indices.size() * sizeof(uint32_t), vk::BufferUsageFlagBits::eIndexBuffer);

  StaticMesh staticMesh{
      std::move(vertexBuffer),
      std::move(indexBuffer),
      static_cast<uint32_t>(indices.size())
  };

  Material meshMaterial;
  meshMaterial.colorTexture = GetTexture(textureFiles.color.value_or(DefaultTextureFile));
  meshMaterial.metallicTexture = GetTexture(textureFiles.metallic.value_or(DefaultTextureFile));
  meshMaterial.roughnessTexture = GetTexture(textureFiles.rougness.value_or(DefaultTextureFile));
  meshMaterial.normalsTexture = GetTexture(textureFiles.normals.value_or(DefaultTextureFile));

  return { std::move(staticMesh), meshMaterial };
}

Image* AssetStorage::LoadTexture(const std::string& textureFile)
{
  int x, y, n;
  int channels_in_file;
  int force_rgba = 4;

  stbi_uc* uc = stbi_load(textureFile.c_str(), &x, &y, &channels_in_file, force_rgba);
  assert(uc);

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

  textures.insert({ textureFile, std::move(img) });

  return &textures.at(textureFile);
}

Image* AssetStorage::GetTexture(const std::string& textureFile)
{
  auto it = textures.find(textureFile);

  if (it != textures.end())
    return &it->second;

  return LoadTexture(textureFile);
}