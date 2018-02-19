
#include "graphic/gfx_precompiled.h"

#include "graphic/gfx_point_light_component.h"

namespace Gfx
{
    CPointLightComponent::CPointLightComponent()
        : m_TextureSMPtr ()
        , m_TextureRSMPtr()
        , m_CameraPtr    (0)
        , m_ShadowmapSize(0)
        , m_TimeStamp    (static_cast<Base::U64>(-1))
    {

    }

    // -----------------------------------------------------------------------------

    CPointLightComponent::~CPointLightComponent()
    {
        m_TextureSMPtr = 0;
        m_CameraPtr    = 0;
    }

    // -----------------------------------------------------------------------------

    CTextureSetPtr CPointLightComponent::GetTextureSMSet() const
    {
        return m_TextureSMPtr;
    }

    // -----------------------------------------------------------------------------

    CTextureSetPtr CPointLightComponent::GetTextureRSMSet() const
    {
        return m_TextureRSMPtr;
    }

    // -----------------------------------------------------------------------------

    CCameraPtr CPointLightComponent::GetCamera() const
    {
        return m_CameraPtr;
    }

    // -----------------------------------------------------------------------------

    unsigned int CPointLightComponent::GetShadowmapSize() const
    {
        return m_ShadowmapSize;
    }

    // -----------------------------------------------------------------------------

    Base::U64 CPointLightComponent::GetTimeStamp() const
    {
        return m_TimeStamp;
    }
} // namespace Gfx