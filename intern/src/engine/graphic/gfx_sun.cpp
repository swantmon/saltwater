
#include "engine/engine_precompiled.h"

#include "engine/graphic/gfx_sun.h"

namespace Gfx
{
    CSun::CSun()
        : m_TextureSMPtr(0)
        , m_CameraPtr   (0)
        , m_TimeStamp   (static_cast<Base::U64>(-1))
    {

    }

    // -----------------------------------------------------------------------------

    CSun::~CSun()
    {
        m_TextureSMPtr = 0;
        m_CameraPtr    = 0;
    }

    // -----------------------------------------------------------------------------

    CTexturePtr CSun::GetShadowMapPtr() const
    {
        return m_TextureSMPtr;
    }

    // -----------------------------------------------------------------------------

    CCameraPtr CSun::GetCamera() const
    {
        return m_CameraPtr;
    }

    // -----------------------------------------------------------------------------

    Base::U64 CSun::GetTimeStamp() const
    {
        return m_TimeStamp;
    }
} // namespace Gfx