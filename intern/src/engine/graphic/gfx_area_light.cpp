
#include "engine/engine_precompiled.h"

#include "engine/graphic/gfx_area_light.h"

namespace Gfx
{
    CAreaLight::CAreaLight()
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

    CAreaLight::~CAreaLight()
    {
    }

    // -----------------------------------------------------------------------------

    CBufferPtr CAreaLight::GetPlaneIndexBuffer() const
    {
        return m_PlaneIndexBufferPtr;
    }

    // -----------------------------------------------------------------------------

    CBufferPtr CAreaLight::GetPlaneVertexBuffer() const
    {
        return m_PlaneVertexBufferSetPtr;
    }

    // -----------------------------------------------------------------------------

    glm::vec4 CAreaLight::GetDirectionX() const
    {
        return m_DirectionX;
    }

    // -----------------------------------------------------------------------------

    glm::vec4 CAreaLight::GetDirectionY() const
    {
        return m_DirectionY;
    }

    // -----------------------------------------------------------------------------

    glm::vec4 CAreaLight::GetPlane() const
    {
        return m_Plane;
    }

    // -----------------------------------------------------------------------------

    float CAreaLight::GetHalfWidth() const
    {
        return m_HalfWidth;
    }

    // -----------------------------------------------------------------------------

    float CAreaLight::GetHalfHeight() const
    {
        return m_HalfHeight;
    }

    // -----------------------------------------------------------------------------

    CTexturePtr CAreaLight::GetTexturePtr() const
    {
        return m_TexturePtr;
    }

    // -----------------------------------------------------------------------------

    CTexturePtr CAreaLight::GetFilteredTexturePtr() const
    {
        return m_FilteredTexturePtr;
    }

    // -----------------------------------------------------------------------------

    bool CAreaLight::HasTexture() const
    {
        return m_TexturePtr != 0 && m_FilteredTexturePtr != 0;
    }
} // namespace Gfx