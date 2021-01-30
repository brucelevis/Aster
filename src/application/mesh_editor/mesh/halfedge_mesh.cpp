#include "halfedge_mesh.h"

#include <map>
#include <set>
#include <tuple>

namespace Editor
{
  glm::vec3 HalfedgeMesh::Face::GetNormal() const
  {
    const auto ab = m_SomeHalfedge->GetDirection();
    const auto bc = m_SomeHalfedge->GetNext()->GetDirection();

    return glm::normalize(glm::cross(bc, ab));
  }

  std::vector<HalfedgeMesh::FaceRef> HalfedgeMesh::Vertex::GetFacesAround() const
  {
    const auto hfBegin = m_OutHalfedge;

    std::vector<FaceRef> faces;

    auto hf = hfBegin;
    while (hf->GetNext()->GetTwin() != hfBegin)
    {
      faces.push_back(hf->GetFace());
      hf = hf->GetNext()->GetTwin();
    }

    return faces;
  }

  glm::vec3 HalfedgeMesh::Vertex::GetAverageNormal() const
  {
    const std::vector<FaceRef> facesAround = GetFacesAround();

    glm::vec3 n{ 0,0,0 };
    for (const auto& f : facesAround)
    {
      if (f->IsBorder() == false)
        n += f->GetNormal();
    }

    if (n == glm::vec3{ 0,0,0 })
      std::printf("warning: there is no true face around vertex with id(%lld)\n", m_Id);

    return glm::normalize(n);
  }

  bool operator<(const HalfedgeMesh::VertexRef& l, const HalfedgeMesh::VertexRef& r)
  {
    return l->GetId() < r->GetId();
  }

  bool operator<(const HalfedgeMesh::HalfedgeCRef& l, const HalfedgeMesh::HalfedgeCRef& r)
  {
    return l->GetId() < r->GetId();
  }

  HalfedgeMesh::Vertex::Vertex(uint64_t id)
    : m_Id(id)
    , m_Position({0,0,0})
  {
  }

  HalfedgeMesh::Edge::Edge(uint64_t id)
    : m_Id(id)
  {
  }

  HalfedgeMesh::Face::Face(uint64_t id)
    : m_Id(id)
  {
  }

  HalfedgeMesh::Halfedge::Halfedge(uint64_t id)
    : m_Id(id)
  {
  }

  glm::vec3 HalfedgeMesh::Halfedge::GetDirection() const
  {
    return glm::normalize(m_Next->GetParentVertex()->GetCenter() - m_ParentVertex->GetCenter());
  }

  std::string HalfedgeMesh::ConstructFromPolygons(const std::vector<std::vector<uint32_t>> polygons, const std::vector<glm::vec3>& vertices)
  {
    std::map<std::set<VertexRef>, EdgeRef> verticesToEdgeMap;
    std::map<uint32_t, VertexRef> idToVertexMap;
    std::vector<std::vector<HalfedgeRef>> halfedgesInPolygon;
    halfedgesInPolygon.resize(polygons.size());

    for (size_t iPolygon = 0; iPolygon < polygons.size(); ++iPolygon)
    {
      const std::vector<uint32_t>& polygon = polygons[iPolygon];

      if (polygon.size() < 3)
        return "polygon has less than 3 vertices.";

      FaceRef f = NewFace();

      std::vector<HalfedgeRef>& hfs = halfedgesInPolygon[iPolygon];

      for (uint32_t id : polygon)
      {
        VertexRef v;
        auto vIt = idToVertexMap.find(id);
        if (vIt != idToVertexMap.end())
        {
          v = vIt->second;
        }
        else
        {
          v = NewVertex();
          v->m_Position = vertices[id];
          idToVertexMap.insert({ id, v });
        }

        HalfedgeRef hf = NewHalfedge();
        hf->m_ParentVertex = v;
        hf->m_Face = f;
        hf->m_Twin = m_Halfedges.end();

        hfs.push_back(hf);
      }

      f->m_SomeHalfedge = hfs[0];

      for (int i = 0; i < polygon.size(); ++i)
      {
        const size_t id = i;
        const size_t nextId = (i == polygon.size() - 1) ? 0 : i + 1;

        auto& hf = hfs[id];
        const auto& hfNext = hfs[nextId];

        hf->m_Next = hfNext;

        auto& v = idToVertexMap.at(polygon[id]);
        const auto& vNext = idToVertexMap.at(polygon[nextId]);

        const auto it = verticesToEdgeMap.find({ v,vNext });
        if (it != verticesToEdgeMap.end())
        {
          auto edge = it->second;
          auto hfTwin = edge->GetSomeHalfedge();
          hf->m_Twin = hfTwin;
          hfTwin->m_Twin = hf;
          hf->m_Edge = edge;
        }
        else
        {
          auto edge = NewEdge();
          verticesToEdgeMap.insert(
            {
              { v,vNext },
              edge 
            }
          );

          edge->m_SomeHalfedge = hf;
          hf->m_Edge = edge;

          v->m_OutHalfedge = hf;
        }
      }
    }

    for (size_t iPolygon = 0; iPolygon < polygons.size(); ++iPolygon)
    {
      std::vector<HalfedgeRef>& hfs = halfedgesInPolygon[iPolygon];

      for (auto& hf : hfs)
      {
        if (hf->m_Twin == m_Halfedges.end())
        {
          std::vector<HalfedgeRef> hfsWithBoundingTwin = CollectLoopOfBoundingHalfedges(hf);
          if (hfsWithBoundingTwin.size() < 3)
            return "bounding face has less than 3 edges.";

          FaceRef boundingFace = NewFace();

          std::vector<HalfedgeRef> boundingHfs;
          boundingHfs.reserve(hfsWithBoundingTwin.size());

          for (HalfedgeRef& hf : hfsWithBoundingTwin)
          {
            HalfedgeRef twin = NewHalfedge();
            hf->m_Twin = twin;

            twin->m_IsBorder = true;
            twin->m_Twin = hf;
            twin->m_Edge = hf->m_Edge;
            twin->m_ParentVertex = hf->GetNext()->m_ParentVertex;
            twin->m_Face = boundingFace;

            boundingHfs.push_back(twin);
          }

          for (size_t iBoundingHf = 0; iBoundingHf < boundingHfs.size(); ++iBoundingHf)
          {
            const size_t nextId = (iBoundingHf == boundingHfs.size() - 1) ? 0 : iBoundingHf + 1;
            boundingHfs[iBoundingHf]->m_Next = boundingHfs[nextId];
          }

          boundingFace->m_SomeHalfedge = boundingHfs[0];
        }
      }
    }

    std::string err = Validate();
    if (err == "")
      m_IsRebuildRequired = true;

    return err;
  }

  std::string HalfedgeMesh::Validate() const
  {
    std::set<HalfedgeCRef> setOfNextHfs;
    std::map<uint64_t, int> edgeLinksCount;
    std::set<HalfedgeRef> twinsSet;

    for (HalfedgeCRef hf = m_Halfedges.cbegin(); hf != m_Halfedges.cend(); ++hf)
    {
      if (setOfNextHfs.find(hf->m_Next) != setOfNextHfs.end())
        return std::string("halfedge with id") + std::to_string(hf->m_Next->m_Id) + " is pointed as `next` by multiple halfedges.";

      setOfNextHfs.insert(hf->m_Next);
    }

    for (HalfedgeCRef hf = m_Halfedges.cbegin(); hf != m_Halfedges.cend(); ++hf)
    {
      if (setOfNextHfs.find(hf) == setOfNextHfs.end())
        return std::string("halfedge with id") + std::to_string(hf->m_Id) + "is not pointed by any halfedges";

      if (hf->m_Twin == hf)
        return std::string("halfedge(") + std::to_string(hf->m_Id) + ")'s twin is itself.";

      if (hf->m_Twin->m_Twin != hf)
        return std::string("halfedge(") + std::to_string(hf->m_Id) + ")'s twin's twin is not an original halfedge.";

      auto it = twinsSet.find(hf->m_Twin);
      if (it != twinsSet.end())
        return std::string("halfedge(") + std::to_string(hf->m_Twin->m_Id) + ") is a twin for more than one Halfedge.";

      ++edgeLinksCount[hf->m_Edge->m_Id];
    }

    for (const auto& [edgeId, count] : edgeLinksCount)
    {
      if (count != 2)
      {
        return std::string("edge(") + std::to_string(edgeId) + ") is pointed by " + std::to_string(count) + " instead of 2 halfedges(Mesh is not manifold).";
      }
    }

    for (VertexCRef v = m_Vertices.cbegin(); v != m_Vertices.cend(); ++v)
    {
      if (v->m_OutHalfedge->m_ParentVertex != v)
        return std::string("vertex(") + std::to_string(v->m_Id) + ")'s halfedge doesn't point to the vertex.";
    }

    for (FaceCRef f = m_Faces.cbegin(); f != m_Faces.cend(); ++f)
    {
      if (f->m_SomeHalfedge->m_Face != f)
        return std::string("face(") + std::to_string(f->m_Id) + ")'s linked hanfledge's face is not the same face.";
    }

    for (EdgeCRef e = m_Edges.cbegin(); e != m_Edges.cend(); ++e)
    {
      if (e->m_SomeHalfedge->m_Edge != e)
        return std::string("edge(") + std::to_string(e->m_Id) + ")'s linked hanfledge's edge is not the same edge.";
    }

    return "";
  }

  std::string HalfedgeMesh::RebuildStaticMesh()
  {
    if (m_Faces.empty())
      return "there are no faces inside halfedge mesh.";

    m_StaticMeshVertices.clear();
    m_StaticMeshIndices.clear();

    std::map<VertexRef, size_t> vertexToIndexInBufffer;

    for (FaceCRef f = m_Faces.begin(); f != m_Faces.cend(); ++f)
    {
      if (f->IsBorder())
        continue;

      HalfedgeCRef beginHf = f->m_SomeHalfedge;
      HalfedgeCRef hf = beginHf;

      std::vector<size_t> idsInVertexBuffer;

      do
      {
        auto it = vertexToIndexInBufffer.find(hf->m_ParentVertex);
        if (it != vertexToIndexInBufffer.end())
        {
          idsInVertexBuffer.push_back(it->second);
        }
        else
        {
          const size_t id = m_StaticMeshVertices.size();
          
          const glm::vec3 position = hf->m_ParentVertex->m_Position;
          const glm::vec3 normal = hf->m_ParentVertex->GetAverageNormal();

          m_StaticMeshVertices.push_back(
            StaticMeshVertex{
              position,
              normal
            }
          );

          vertexToIndexInBufffer.insert({ hf->m_ParentVertex, id });
          idsInVertexBuffer.push_back(id);
        }

        hf = hf->m_Next;
      } while (hf != beginHf);

      size_t beginId = idsInVertexBuffer[0];
      for (size_t i = 1; i < idsInVertexBuffer.size() - 1; ++i)
      {
        m_StaticMeshIndices.push_back(beginId);
        m_StaticMeshIndices.push_back(idsInVertexBuffer[i]);
        m_StaticMeshIndices.push_back(idsInVertexBuffer[i+1]);
      }
    }

    m_IsRebuildRequired = false;

    return "";
  }

  HalfedgeMesh::HalfedgeRef HalfedgeMesh::FindNextBoundingHalfedge(HalfedgeMesh::HalfedgeRef beginHf) const
  {
    using HalfedgeRef = HalfedgeMesh::HalfedgeRef;

    auto hf = beginHf->GetNext();
    if (hf->GetTwin() == m_Halfedges.end())
      return hf;

    while (hf->GetTwin() != m_Halfedges.end())
    {
      hf = hf->GetTwin()->GetNext();
    }

    return hf;
  }

  std::vector<HalfedgeMesh::HalfedgeRef> HalfedgeMesh::CollectLoopOfBoundingHalfedges(HalfedgeMesh::HalfedgeRef begin) const
  {
    std::vector<HalfedgeMesh::HalfedgeRef> hfs;

    hfs.push_back(begin);

    auto hf = FindNextBoundingHalfedge(begin);
    while (hf != begin)
    {
      hfs.push_back(hf);
      hf = FindNextBoundingHalfedge(hf);
    }

    return hfs;
  }
}