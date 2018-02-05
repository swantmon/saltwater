
#include "graphic/gfx_precompiled.h"

#include "graphic/gfx_area_light_facet.h"

namespace Gfx
{
    CAreaLightFacet::CAreaLightFacet()
        : m_PlaneIndexBufferPtr    (0)
        , m_PlaneVertexBufferSetPtr(0)
        , m_TexturePtr             (0)
        , m_FilteredTexturePtr     (0)
        , m_DirectionX             (0.0f)
        , m_DirectionY             (0.0f)
        , m_Plane                  (0.0f)
        , m_HalfWidth              (0.0f)
        , m_HalfHeight             (0.0f)
        , m_TimeStamp              (static_cast<Base::U64>(-1))
    {

    }

    // -----------------------------------------------------------------------------

    CAreaLightFacet::~CAreaLightFacet()
    {
    }

    // -----------------------------------------------------------------------------

    CBufferPtr CAreaLightFacet::GetPlaneIndexBuffer() const
    {
        return m_PlaneIndexBufferPtr;
    }

    // -----------------------------------------------------------------------------

    CBufferPtr CAreaLightFacet::GetPlaneVertexBuffer() const
    {
        return m_PlaneVertexBufferSetPtr;
    }

    // -----------------------------------------------------------------------------

    glm::vec4 CAreaLightFacet::GetDirectionX() const
    {
        return m_DirectionX;
    }

    // -----------------------------------------------------------------------------

    glm::vec4 CAreaLightFacet::GetDirectionY() const
    {
        return m_DirectionY;
    }

    // -----------------------------------------------------------------------------

    glm::vec4 CAreaLightFacet::GetPlane() const
    {
        return m_Plane;
    }

    // -----------------------------------------------------------------------------

    float CAreaLightFacet::GetHalfWidth() const
    {
        return m_HalfWidth;
    }

    // -----------------------------------------------------------------------------

    float CAreaLightFacet::GetHalfHeight() const
    {
        return m_HalfHeight;
    }

    // -----------------------------------------------------------------------------

    CTexturePtr CAreaLightFacet::GetTexturePtr() const
    {
        return m_TexturePtr;
    }

    // -----------------------------------------------------------------------------

    CTexturePtr CAreaLightFacet::GetFilteredTexturePtr() const
    {
        return m_FilteredTexturePtr;
    }

    // -----------------------------------------------------------------------------

    bool CAreaLightFacet::HasTexture() const
    {
        return m_TexturePtr != 0 && m_FilteredTexturePtr != 0;
    }
} // namespace Gfx