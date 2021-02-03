#include "mesh_generation.h"

#include <unordered_map>
#include <iterator>

namespace glm
{
  struct VecComp
  {
    size_t operator()(const vec3& v)const
    {
      return std::hash<int>()(v.x) ^ std::hash<int>()(v.y) ^ std::hash<int>()(v.z);
    }

    bool operator()(const vec3& a, const vec3& b)const
    {
      return a.x == b.x &&
        a.y == b.y &&
        a.z == b.z;
    }
  };
}

namespace
{
  class SphereBuilder
  {
    struct Face
    {
      uint32_t v0, v1, v2;

      inline std::vector<uint32_t> ToIndices() const
      {
        return { v0,v1,v2 };
      }
    };

    typedef std::unordered_map<glm::vec3, uint32_t, glm::VecComp, glm::VecComp> VertexToIndexMap;

  public:
    static std::tuple<std::vector<glm::vec3>, std::vector<uint32_t>> Build(int splitFacesNTimes, float radius)
    {
      glm::vec3 A(-1, 0, -1); //0
      glm::vec3 B(-1, 0, 1);  //1
      glm::vec3 C(1, 0, 1);   //2
      glm::vec3 D(1, 0, -1);  //3
      glm::vec3 E(0, -1, 0);  //4
      glm::vec3 F(0, 1, 0);   //5

      std::vector<glm::vec3> vertices;
      VertexToIndexMap vertexToIdInBuffer;

      for (auto& v : { A,B,C,D,E,F })
      {
        const uint32_t id = static_cast<uint32_t>(vertices.size());
        vertices.push_back(v);
        vertexToIdInBuffer.insert({ v, id });
      }

      std::vector<Face> faces
      {
        {4, 3, 0},
        {4, 2, 3},
        {4, 1, 2},
        {4, 0, 1},

        {5, 0, 3},
        {5, 3, 2},
        {5, 2, 1},
        {5, 1, 0}
      };

      for (int i = 0; i < splitFacesNTimes; ++i)
      {
        faces = SplitFaces(faces, vertices, vertexToIdInBuffer);
      }

      std::vector<uint32_t> indices;
      indices.reserve(faces.size() * 3);
      for (const Face& face : faces)
      {
        const std::vector<uint32_t> faceIds = face.ToIndices();
        std::copy(faceIds.begin(), faceIds.end(), std::back_inserter(indices));
      }

      for (glm::vec3& v : vertices)
      {
        glm::vec3 dir = glm::normalize(v);
        v = dir * radius;
      }

      return { vertices, indices };
    }

  private:
    static std::vector<Face> SplitFaces(const std::vector<Face>& faces, std::vector<glm::vec3>& vertices, VertexToIndexMap& vertexToIdInBuffer)
    {
      std::vector<Face> newFaces;

      for (const auto& face : faces)
      {
        std::vector<Face> fs = SplitFace(face, vertices, vertexToIdInBuffer);
        std::copy(fs.begin(), fs.end(), std::back_inserter(newFaces));
      }

      return newFaces;
    }

    static std::vector<Face> SplitFace(const Face& face, std::vector<glm::vec3>& vertices, VertexToIndexMap& vertexToIdInBuffer)
    {
      const glm::vec3& v0 = vertices[face.v0];
      const glm::vec3& v1 = vertices[face.v1];
      const glm::vec3& v2 = vertices[face.v2];

      const glm::vec3 v0v1_center_v = v0 + (v1 - v0) / 2.0f;
      const glm::vec3 v1v2_center_v = v1 + (v2 - v1) / 2.0f;
      const glm::vec3 v2v0_center_v = v2 + (v0 - v2) / 2.0f;

      for (const auto& v : { v0v1_center_v, v1v2_center_v, v2v0_center_v })
      {
        const auto it = vertexToIdInBuffer.find(v);
        if (it == vertexToIdInBuffer.end())
        {
          const size_t id = vertices.size();
          vertices.push_back(v);
          vertexToIdInBuffer.insert({ v, id });
        }
      }

      const uint32_t v0v1_center = vertexToIdInBuffer.at(v0v1_center_v);
      const uint32_t v1v2_center = vertexToIdInBuffer.at(v1v2_center_v);
      const uint32_t v2v0_center = vertexToIdInBuffer.at(v2v0_center_v);

      return std::vector<Face>{
        {face.v0, v0v1_center, v2v0_center},
        { v0v1_center, face.v1, v1v2_center },
        { v2v0_center, v1v2_center, face.v2 },
        { v0v1_center, v1v2_center, v2v0_center }
      };
    }
  };
}

namespace Utils
{
  std::vector<glm::vec3> GenerateWirePlane(float r, size_t nLines)
  {
    std::vector<glm::vec3> vertices;
    vertices.reserve(2 * nLines);
    std::vector<uint32_t> indices;
    indices.reserve(2 * nLines);


    float dl = 2*r / nLines;

    uint32_t i = 0;
    for (float x = -r; x <= r; x += dl)
    {
      vertices.push_back({ x, 0.0f, -r });
      vertices.push_back({ x, 0.0f, +r });
    }

    for (float z = -r; z <= r; z += dl)
    {
      vertices.push_back({ -r, 0.0f, z });
      vertices.push_back({ +r, 0.0f, z });
    }

    return std::move(vertices);
  }

  std::tuple<std::vector<glm::vec3>, std::vector<uint32_t>> GenerateSphere(int splitFacesNTimes, float radius)
  {
    return SphereBuilder::Build(splitFacesNTimes, radius);
  }
}