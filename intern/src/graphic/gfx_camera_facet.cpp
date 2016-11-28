
#include "graphic/gfx_precompiled.h"

#include "graphic/gfx_camera_facet.h"

namespace Gfx
{
    CCameraActorFacet::CCameraActorFacet()
        : m_BackgroundTexture2DPtr(0)
        , m_BackgroundTextureSetPtr(0)
        , m_TimeStamp(static_cast<Base::U64>(-1))
    {

    }

    // -----------------------------------------------------------------------------

    CCameraActorFacet::~CCameraActorFacet()
    {
        m_BackgroundTexture2DPtr  = 0;
        m_BackgroundTextureSetPtr = 0;
    }

    // -----------------------------------------------------------------------------

    void CCameraActorFacet::SetBackgroundTexture2D(CTexture2DPtr _Texture2DPtr)
    {
        m_BackgroundTexture2DPtr = _Texture2DPtr;
    }

    // -----------------------------------------------------------------------------

    CTexture2DPtr CCameraActorFacet::GetBackgroundTexture2D()
    {
        return m_BackgroundTexture2DPtr;
    }

    // -----------------------------------------------------------------------------

    void CCameraActorFacet::SetBackgroundTextureSet(CTextureSetPtr _TextureSetPtr)
    {
        m_BackgroundTextureSetPtr = _TextureSetPtr;
    }

    // -----------------------------------------------------------------------------

    CTextureSetPtr CCameraActorFacet::GetBackgroundTextureSet()
    {
        return m_BackgroundTextureSetPtr;
    }

    // -----------------------------------------------------------------------------

    void CCameraActorFacet::SetTimeStamp(Base::U64 _TimeStamp)
    {
        m_TimeStamp = _TimeStamp;
    }

    // -----------------------------------------------------------------------------

    Base::U64 CCameraActorFacet::GetTimeStamp()
    {
        return m_TimeStamp;
    }
} // namespace Gfx