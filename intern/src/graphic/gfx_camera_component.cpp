
#include "graphic/gfx_precompiled.h"

#include "graphic/gfx_camera_component.h"

namespace Gfx
{
    CCameraComponent::CCameraComponent()
        : m_BackgroundTexture2DPtr(0)
        , m_BackgroundTextureSetPtr(0)
        , m_TimeStamp(static_cast<Base::U64>(-1))
    {

    }

    // -----------------------------------------------------------------------------

    CCameraComponent::~CCameraComponent()
    {
        m_BackgroundTexture2DPtr  = 0;
        m_BackgroundTextureSetPtr = 0;
    }

    // -----------------------------------------------------------------------------

    void CCameraComponent::SetBackgroundTexture2D(CTexturePtr _Texture2DPtr)
    {
        m_BackgroundTexture2DPtr = _Texture2DPtr;
    }

    // -----------------------------------------------------------------------------

    CTexturePtr CCameraComponent::GetBackgroundTexture2D()
    {
        return m_BackgroundTexture2DPtr;
    }

    // -----------------------------------------------------------------------------

    void CCameraComponent::SetBackgroundTextureSet(CTextureSetPtr _TextureSetPtr)
    {
        m_BackgroundTextureSetPtr = _TextureSetPtr;
    }

    // -----------------------------------------------------------------------------

    CTextureSetPtr CCameraComponent::GetBackgroundTextureSet()
    {
        return m_BackgroundTextureSetPtr;
    }

    // -----------------------------------------------------------------------------

    void CCameraComponent::SetTimeStamp(Base::U64 _TimeStamp)
    {
        m_TimeStamp = _TimeStamp;
    }

    // -----------------------------------------------------------------------------

    Base::U64 CCameraComponent::GetTimeStamp()
    {
        return m_TimeStamp;
    }
} // namespace Gfx