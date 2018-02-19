
#include "graphic/gfx_precompiled.h"

#include "graphic/gfx_area_light_component.h"

namespace Gfx
{
    CAreaLightComponent::CAreaLightComponent()
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

    CAreaLightComponent::~CAreaLightComponent()
    {
    }

    // -----------------------------------------------------------------------------

    CBufferPtr CAreaLightComponent::GetPlaneIndexBuffer() const
    {
        return m_PlaneIndexBufferPtr;
    }

    // -----------------------------------------------------------------------------

    CBufferPtr CAreaLightComponent::GetPlaneVertexBuffer() const
    {
        return m_PlaneVertexBufferSetPtr;
    }

    // -----------------------------------------------------------------------------

    glm::vec4 CAreaLightComponent::GetDirectionX() const
    {
        return m_DirectionX;
    }

    // -----------------------------------------------------------------------------

    glm::vec4 CAreaLightComponent::GetDirectionY() const
    {
        return m_DirectionY;
    }

    // -----------------------------------------------------------------------------

    glm::vec4 CAreaLightComponent::GetPlane() const
    {
        return m_Plane;
    }

    // -----------------------------------------------------------------------------

    float CAreaLightComponent::GetHalfWidth() const
    {
        return m_HalfWidth;
    }

    // -----------------------------------------------------------------------------

    float CAreaLightComponent::GetHalfHeight() const
    {
        return m_HalfHeight;
    }

    // -----------------------------------------------------------------------------

    CTexturePtr CAreaLightComponent::GetTexturePtr() const
    {
        return m_TexturePtr;
    }

    // -----------------------------------------------------------------------------

    CTexturePtr CAreaLightComponent::GetFilteredTexturePtr() const
    {
        return m_FilteredTexturePtr;
    }

    // -----------------------------------------------------------------------------

    bool CAreaLightComponent::HasTexture() const
    {
        return m_TexturePtr != 0 && m_FilteredTexturePtr != 0;
    }
} // namespace Gfx