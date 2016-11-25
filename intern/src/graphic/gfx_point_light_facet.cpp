
#include "graphic/gfx_precompiled.h"

#include "graphic/gfx_point_light_facet.h"

namespace Gfx
{
    CPointLightFacet::CPointLightFacet()
        : m_TextureSMPtr()
        , m_CameraPtr   (0)
        , m_TimeStamp   (static_cast<Base::U64>(-1))
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

    CCameraPtr CPointLightFacet::GetCamera() const
    {
        return m_CameraPtr;
    }

    // -----------------------------------------------------------------------------

    Base::U64 CPointLightFacet::GetTimeStamp() const
    {
        return m_TimeStamp;
    }
} // namespace Gfx