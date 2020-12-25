#include "asset_storage.h"
#include <engine/rendering/vulkan/core.h>
#include <engine/rendering/components/static_mesh_component.h>

#include <array>

namespace
{
  const std::array cubeVertices{
    StaticMesh::Vertex{
      {-0.5f, -0.5f, 0.5f},
      {1.0f, 0.0f, 0.0f},
    },
    StaticMesh::Vertex{
      {0.5f, -0.5f, 0.5f},
      {1.0f, 0.0f, 0.0f},
    },
    StaticMesh::Vertex{
      {0.5f, 0.5f, 0.5f},
      {1.0f, 0.0f, 0.0f},
    },
    StaticMesh::Vertex{
      {-0.5f, 0.5f, 0.5f},
      {1.0f, 0.0f, 0.0f},
    },
    StaticMesh::Vertex{
      {0.5f, 0.5f, -0.5f},
      {1.0f, 1.0f, 0.0f},
    },
    StaticMesh::Vertex{
      {0.5f, -0.5f, -0.5f},
      {1.0f, 1.0f, 0.0f},
    },
    StaticMesh::Vertex{
      {-0.5f, -0.5f, -0.5f},
      {1.0f, 0.0f, 1.0f},
    },
    StaticMesh::Vertex{
      {-0.5f, 0.5f, -0.5f},
      {1.0f, 0.0f, 1.0f},
    },
  };

  const std::array<uint32_t, 36> cubeIndices{
    0,1,2,
    2,3,0,

    0,6,5,
    5,1,0,

    2,1,5,
    5,4,2,

    7,6,0,
    0,3,7,

    3,2,4,
    4,7,3,

    4,5,6,
    6,7,4
  };
}

AssetStorage::AssetStorage(Core& vkCore)
  : vkCore(vkCore)
{
  InitializeBasicMeshes();
}

void AssetStorage::InitializeBasicMeshes()
{
  Buffer cubeVertexBuf = vkCore.AllocateDeviceBuffer(
    reinterpret_cast<const void*>(cubeVertices.data()),
    cubeVertices.size() * sizeof(StaticMesh::Vertex),
    vk::BufferUsageFlagBits::eVertexBuffer
  );

  Buffer cubeIndexBuf = vkCore.AllocateDeviceBuffer(
    reinterpret_cast<const void*>(cubeIndices.data()),
    cubeIndices.size() * sizeof(uint32_t),
    vk::BufferUsageFlagBits::eIndexBuffer
  );

  staticMeshes.insert(
    {
      CUBE_STATIC_MESH,
      StaticMesh{std::move(cubeVertexBuf), std::move(cubeIndexBuf), cubeIndices.size()}
    }
  );
}