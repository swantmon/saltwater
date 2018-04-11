
#include "engine/engine_precompiled.h"

#include "engine/graphic/gfx_point_light.h"

namespace Gfx
{
    CPointLight::CPointLight()
        : m_TextureSMPtr ()
        , m_TextureRSMPtr()
        , m_CameraPtr    (0)
        , m_ShadowmapSize(0)
        , m_TimeStamp    (static_cast<Base::U64>(-1))
    {

    }

    // -----------------------------------------------------------------------------

    CPointLight::~CPointLight()
    {
        m_TextureSMPtr = 0;
        m_CameraPtr    = 0;
    }

    // -----------------------------------------------------------------------------

    CTextureSetPtr CPointLight::GetTextureSMSet() const
    {
        return m_TextureSMPtr;
    }

    // -----------------------------------------------------------------------------

    CTextureSetPtr CPointLight::GetTextureRSMSet() const
    {
        return m_TextureRSMPtr;
    }

    // -----------------------------------------------------------------------------

    CCameraPtr CPointLight::GetCamera() const
    {
        return m_CameraPtr;
    }

    // -----------------------------------------------------------------------------

    unsigned int CPointLight::GetShadowmapSize() const
    {
        return m_ShadowmapSize;
    }

    // -----------------------------------------------------------------------------

    Base::U64 CPointLight::GetTimeStamp() const
    {
        return m_TimeStamp;
    }
} // namespace Gfx