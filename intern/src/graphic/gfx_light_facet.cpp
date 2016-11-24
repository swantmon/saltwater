
#include "graphic/gfx_precompiled.h"

#include "graphic/gfx_light_facet.h"

namespace Gfx
{
    CPointLightFacet::CPointLightFacet()
        : m_RenderContextPtr(0)
        , m_TextureSMPtr    (0)
        , m_TimeStamp       (static_cast<Base::U64>(-1))
    {

    }

    // -----------------------------------------------------------------------------

    CPointLightFacet::~CPointLightFacet()
    {

    }

    // -----------------------------------------------------------------------------

    void CPointLightFacet::SetRenderContext(CRenderContextPtr _RenderContextPtr)
    {
        m_RenderContextPtr = _RenderContextPtr;
    }

    // -----------------------------------------------------------------------------

    CRenderContextPtr CPointLightFacet::GetRenderContext()
    {
        return m_RenderContextPtr;
    }

    // -----------------------------------------------------------------------------

    void CPointLightFacet::SetTextureSMSet(CTextureSetPtr _TextureSMPtr)
    {
        m_TextureSMPtr = _TextureSMPtr;
    }

    // -----------------------------------------------------------------------------

    CTextureSetPtr CPointLightFacet::GetTextureSMSet()
    {
        return m_TextureSMPtr;
    }

    // -----------------------------------------------------------------------------

    void CPointLightFacet::SetTimeStamp(Base::U64 _TimeStamp)
    {
        m_TimeStamp = _TimeStamp;
    }

    // -----------------------------------------------------------------------------

    Base::U64 CPointLightFacet::GetTimeStamp()
    {
        return m_TimeStamp;
    }
} // namespace Gfx

namespace Gfx
{
    CSunLightFacet::CSunLightFacet()
        : m_RenderContextPtr(0)
        , m_TextureSMPtr    (0)
        , m_TimeStamp       (static_cast<Base::U64>(-1))
    {

    }

    // -----------------------------------------------------------------------------

    CSunLightFacet::~CSunLightFacet()
    {

    }

    // -----------------------------------------------------------------------------

    void CSunLightFacet::SetRenderContext(CRenderContextPtr _RenderContextPtr)
    {
        m_RenderContextPtr = _RenderContextPtr;
    }

    // -----------------------------------------------------------------------------

    CRenderContextPtr CSunLightFacet::GetRenderContext()
    {
        return m_RenderContextPtr;
    }

    // -----------------------------------------------------------------------------

    void CSunLightFacet::SetTextureSMSet(CTextureSetPtr _TextureSMPtr)
    {
        m_TextureSMPtr = _TextureSMPtr;
    }

    // -----------------------------------------------------------------------------

    CTextureSetPtr CSunLightFacet::GetTextureSMSet()
    {
        return m_TextureSMPtr;
    }

    // -----------------------------------------------------------------------------

    void CSunLightFacet::SetTimeStamp(Base::U64 _TimeStamp)
    {
        m_TimeStamp = _TimeStamp;
    }

    // -----------------------------------------------------------------------------

    Base::U64 CSunLightFacet::GetTimeStamp()
    {
        return m_TimeStamp;
    }
} // namespace Gfx