#pragma once

#include <list>
#include <vector>
#include <string>
#include <optional>

#include <glm/glm.hpp>
#include <tuple>

namespace Editor
{
  struct StaticMeshVertex
  {
    glm::vec3 position;
    glm::vec3 normal;
  };

  class HalfedgeMesh
  {
  public:
    class Vertex;
    class Edge;
    class Face;
    class Halfedge;

    typedef std::list<Vertex>::iterator         VertexRef;
    typedef std::list<Vertex>::const_iterator   VertexCRef;

    typedef std::list<Edge>::iterator           EdgeRef;
    typedef std::list<Edge>::const_iterator     EdgeCRef;

    typedef std::list<Halfedge>::iterator       HalfedgeRef;
    typedef std::list<Halfedge>::const_iterator HalfedgeCRef;

    typedef std::list<Face>::iterator           FaceRef;
    typedef std::list<Face>::const_iterator     FaceCRef;

    class Vertex
    {
      friend HalfedgeMesh;
    public:
      inline HalfedgeRef GetOutputHalfedge()
      {
        return m_OutHalfedge;
      }

      inline HalfedgeCRef GetOutputHalfedge() const
      {
        return m_OutHalfedge;
      }

      inline glm::vec3 GetCenter() const
      {
        return m_Position;
      }

      inline void SetOutputHalfedge(const HalfedgeRef& hf)
      {
        m_OutHalfedge = hf;
      }

      inline unsigned int GetId() const
      {
        return m_Id;
      }

      std::vector<FaceRef> GetFacesAround() const;

      glm::vec3 GetAverageNormal() const;

    private:
      Vertex(uint64_t id);

    private:
      uint64_t m_Id = 0;
      glm::vec3 m_Position;
      HalfedgeRef m_OutHalfedge;
    };

    class Edge
    {
      friend HalfedgeMesh;
    public:

      inline unsigned int GetId() const
      {
        return m_Id;
      }

      inline void SetSomeHalfedge(const HalfedgeRef& r)
      {
        m_SomeHalfedge = r;
      }

      inline HalfedgeRef GetSomeHalfedge()
      {
        return m_SomeHalfedge;
      }

      inline HalfedgeCRef GetSomeHalfedge() const
      {
        return m_SomeHalfedge;
      }

    private:
      Edge(uint64_t id);

      uint64_t m_Id = 0;
      HalfedgeRef m_SomeHalfedge;
    };

    class Face
    {
      friend HalfedgeMesh;
    public:
      Face(uint64_t id);

      glm::vec3 GetNormal() const;

      inline HalfedgeRef GetSomeHalfedge()
      {
        return m_SomeHalfedge;
      }

      inline HalfedgeCRef GetSomeHalfedge() const
      {
        return m_SomeHalfedge;
      }

      inline void SetSomeHalfedge(const HalfedgeRef r)
      {
        m_SomeHalfedge = r;
      }

      inline bool IsBorder() const
      {
        return m_SomeHalfedge->IsBorder();
      }

      inline unsigned int GetId() const
      {
        return m_Id;
      }

    private:
      uint64_t m_Id = 0;
      HalfedgeRef m_SomeHalfedge;
    };

    class Halfedge
    {
      friend HalfedgeMesh;
    public:
      Halfedge(uint64_t id);

      inline void Setup(const HalfedgeRef& twin,
                 const HalfedgeRef& next,
                 const EdgeRef& edge,
                 const FaceRef& face,
                 const VertexRef& parentVertex)
      {
        m_Twin = twin;
        m_Next = next;
        m_Edge = edge;
        m_Face = face;
        m_ParentVertex = parentVertex;
      }

      inline HalfedgeRef GetTwin()
      {
        return m_Twin;
      }

      inline HalfedgeCRef GetTwin() const
      {
        return m_Twin;
      }

      inline HalfedgeRef GetNext()
      {
        return m_Next;
      }

      inline HalfedgeCRef GetNext() const
      {
        return m_Next;
      }

      inline EdgeRef GetEdge()
      {
        return m_Edge;
      }

      inline EdgeCRef GetEdge() const
      {
        return m_Edge;
      }

      inline FaceRef GetFace()
      {
        return m_Face;
      }

      inline FaceCRef GetFace() const
      {
        return m_Face;
      }

      inline VertexRef GetParentVertex()
      {
        return m_ParentVertex;
      }

      inline VertexCRef GetParentVertex() const
      {
        return m_ParentVertex;
      }

      inline bool IsBorder() const
      {
        return m_IsBorder;
      }

      inline unsigned int GetId() const
      {
        return m_Id;
      }

      glm::vec3 GetDirection() const;

    private:
      uint64_t m_Id = 0;
      bool m_IsBorder = false;
      HalfedgeRef m_Twin;
      HalfedgeRef m_Next;
      EdgeRef m_Edge;
      FaceRef m_Face;
      VertexRef m_ParentVertex;
    };

  public:
    std::string Validate() const;
    std::string ConstructFromPolygons(const std::vector<std::vector<uint32_t>> polygons, const std::vector<glm::vec3>& vertices);
    std::string RebuildStaticMesh();

    inline VertexRef NewVertex()
    {
      return m_Vertices.insert(m_Vertices.end(), Vertex{ freeId++ });
    }

    inline EdgeRef NewEdge()
    {
      return m_Edges.insert(m_Edges.end(), Edge{ freeId++ });
    }

    inline FaceRef NewFace()
    {
      return m_Faces.insert(m_Faces.end(), Face{ freeId++ });
    }

    inline HalfedgeRef NewHalfedge()
    {
      return m_Halfedges.insert(m_Halfedges.end(), Halfedge{ freeId++ });
    }

    inline size_t GetVertexCount() const
    {
      return m_Vertices.size();
    }

    inline size_t GetEdgesCount() const
    {
      return m_Edges.size();
    }

    inline size_t GetFacesCount() const
    {
      return m_Faces.size();
    }

    inline size_t GetHalfedgesCount() const
    {
      return m_Halfedges.size();
    }

    inline void MarkToRebuild()
    {
      m_IsRebuildRequired = true;
    }

    inline bool IsRebuildRequired() const
    {
      return m_IsRebuildRequired;
    }

    inline std::tuple<const std::vector<StaticMeshVertex>&, const std::vector<uint32_t>&> GetStaticMeshData() const
    {
      return { m_StaticMeshVertices, m_StaticMeshIndices };
    }

  private:
    HalfedgeRef FindNextBoundingHalfedge(HalfedgeRef beginHf) const;
    std::vector<HalfedgeRef> CollectLoopOfBoundingHalfedges(HalfedgeRef begin) const;

  private:
    uint64_t freeId = 0;
    bool m_IsRebuildRequired = false;

    std::list<Vertex> m_Vertices;
    std::list<Edge> m_Edges;
    std::list<Face> m_Faces;
    std::list<Halfedge> m_Halfedges;


    std::vector<StaticMeshVertex> m_StaticMeshVertices;
    std::vector<uint32_t> m_StaticMeshIndices;
  };
}