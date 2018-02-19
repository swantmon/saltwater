
#include "graphic/gfx_precompiled.h"

#include "graphic/gfx_sun_component.h"

namespace Gfx
{
    CSunComponent::CSunComponent()
        : m_TextureSMPtr(0)
        , m_CameraPtr   (0)
        , m_TimeStamp   (static_cast<Base::U64>(-1))
    {

    }

    // -----------------------------------------------------------------------------

    CSunComponent::~CSunComponent()
    {
        m_TextureSMPtr = 0;
        m_CameraPtr    = 0;
    }

    // -----------------------------------------------------------------------------

    CTexturePtr CSunComponent::GetShadowMapPtr() const
    {
        return m_TextureSMPtr;
    }

    // -----------------------------------------------------------------------------

    CCameraPtr CSunComponent::GetCamera() const
    {
        return m_CameraPtr;
    }

    // -----------------------------------------------------------------------------

    Base::U64 CSunComponent::GetTimeStamp() const
    {
        return m_TimeStamp;
    }
} // namespace Gfx