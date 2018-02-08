
#include "data/data_precompiled.h"

#include "data/data_surface.h"

namespace Dt
{
    CSurface::CSurface()
        : m_Elements        (Position)
        , m_NumberOfIndices (0)
        , m_NumberOfVertices(0)
        , m_pIndices        (0)
        , m_pPositions      (0)
        , m_pNormals        (0)
        , m_pTangents       (0)
        , m_pBitangents     (0)
        , m_pTexCoords      (0)
        , m_pMaterial       (0)
    {
        
    }

    // -----------------------------------------------------------------------------

    CSurface::~CSurface()
    {

    }

    // -----------------------------------------------------------------------------

    void CSurface::SetElements(unsigned int _Elements)
    {
        m_Elements = _Elements;
    }

    // -----------------------------------------------------------------------------

    unsigned int CSurface::GetElements() const
    {
        return m_Elements;
    }

    // -----------------------------------------------------------------------------

    void CSurface::SetNumberOfIndices(unsigned int _NumberOfIndices)
    {
        m_NumberOfIndices = _NumberOfIndices;
    }

    // -----------------------------------------------------------------------------

    unsigned int CSurface::GetNumberOfIndices() const
    {
        return m_NumberOfIndices;
    }

    // -----------------------------------------------------------------------------


    void CSurface::SetNumberOfVertices(unsigned int _NumberOfVertices)
    {
        m_NumberOfVertices = _NumberOfVertices;
    }

    // -----------------------------------------------------------------------------

    unsigned int CSurface::GetNumberOfVertices() const
    {
        return m_NumberOfVertices;
    }

    // -----------------------------------------------------------------------------


    void CSurface::SetIndices(BIndice* _pIndices)
    {
        m_pIndices = _pIndices;
    }

    // -----------------------------------------------------------------------------

    CSurface::BIndice* CSurface::GetIndices()
    {
        return m_pIndices;
    }

    // -----------------------------------------------------------------------------

    const CSurface::BIndice* CSurface::GetIndices() const
    {
        return m_pIndices;
    }

    // -----------------------------------------------------------------------------

    void CSurface::SetPositions(glm::vec3* _pPositions)
    {
        m_pPositions = _pPositions;
    }

    // -----------------------------------------------------------------------------

    glm::vec3* CSurface::GetPositions()
    {
        return m_pPositions;
    }

    // -----------------------------------------------------------------------------

    const glm::vec3* CSurface::GetPositions() const
    {
        return m_pPositions;
    }

    // -----------------------------------------------------------------------------

    void CSurface::SetNormals(glm::vec3* _pNormals)
    {
        m_pNormals = _pNormals;
    }

    // -----------------------------------------------------------------------------

    glm::vec3* CSurface::GetNormals()
    {
        return m_pNormals;
    }

    // -----------------------------------------------------------------------------

    const glm::vec3* CSurface::GetNormals() const
    {
        return m_pNormals;
    }

    // -----------------------------------------------------------------------------

    void CSurface::SetTangents(glm::vec3* _pTangents)
    {
        m_pTangents = _pTangents;
    }

    // -----------------------------------------------------------------------------

    glm::vec3* CSurface::GetTangents()
    {
        return m_pTangents;
    }

    // -----------------------------------------------------------------------------

    const glm::vec3* CSurface::GetTangents() const
    {
        return m_pTangents;
    }

    // -----------------------------------------------------------------------------

    void CSurface::SetBitangents(glm::vec3* _pBitangents)
    {
        m_pBitangents = _pBitangents;
    }

    // -----------------------------------------------------------------------------

    glm::vec3* CSurface::GetBitangents()
    {
        return m_pBitangents;
    }

    // -----------------------------------------------------------------------------

    const glm::vec3* CSurface::GetBitangents() const
    {
        return m_pBitangents;
    }

    // -----------------------------------------------------------------------------

    void CSurface::SetTexCoords(glm::vec2* _pTexCoords)
    {
        m_pTexCoords = _pTexCoords;
    }

    // -----------------------------------------------------------------------------

    glm::vec2* CSurface::GetTexCoords()
    {
        return m_pTexCoords;
    }

    // -----------------------------------------------------------------------------

    const glm::vec2* CSurface::GetTexCoords() const
    {
        return m_pTexCoords;
    }

    // -----------------------------------------------------------------------------

    void CSurface::SetMaterial(CMaterial* _pMaterial)
    {
        m_pMaterial = _pMaterial;
    }

    // -----------------------------------------------------------------------------

    CMaterial* CSurface::GetMaterial()
    {
        return m_pMaterial;
    }

    // -----------------------------------------------------------------------------

    const CMaterial* CSurface::GetMaterial() const
    {
        return m_pMaterial;
    }
} // namespace Dt