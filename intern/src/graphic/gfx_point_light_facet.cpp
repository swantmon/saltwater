
#include "graphic/gfx_precompiled.h"

#include "graphic/gfx_point_light_facet.h"

namespace Gfx
{
    CPointLightFacet::CPointLightFacet()
        : m_TextureSMPtr ()
        , m_TextureRSMPtr()
        , m_CameraPtr    (0)
        , m_ShadowmapSize(0)
        , m_TimeStamp    (static_cast<Base::U64>(-1))
    {

    }

    // -----------------------------------------------------------------------------

    CPointLightFacet::~CPointLightFacet()
    {
        m_TextureSMPtr = 0;
        m_CameraPtr    = 0;
    }

    // -----------------------------------------------------------------------------

    CTextureSetPtr CPointLightFacet::GetTextureSMSet() const
    {
        return m_TextureSMPtr;
    }

    // -----------------------------------------------------------------------------

    CTextureSetPtr CPointLightFacet::GetTextureRSMSet() const
    {
        return m_TextureRSMPtr;
    }

    // -----------------------------------------------------------------------------

    CCameraPtr CPointLightFacet::GetCamera() const
    {
        return m_CameraPtr;
    }

    // -----------------------------------------------------------------------------

    unsigned int CPointLightFacet::GetShadowmapSize() const
    {
        return m_ShadowmapSize;
    }

    // -----------------------------------------------------------------------------

    Base::U64 CPointLightFacet::GetTimeStamp() const
    {
        return m_TimeStamp;
    }
} // namespace Gfx