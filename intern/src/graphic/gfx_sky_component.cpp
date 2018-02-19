
#include "graphic/gfx_precompiled.h"

#include "graphic/gfx_sky_component.h"

namespace Gfx
{
    CSkyComponent::CSkyComponent()
        : m_CubemapPtr   ()
        , m_TimeStamp    (static_cast<Base::U64>(-1))
    {

    }

    // -----------------------------------------------------------------------------

    CSkyComponent::~CSkyComponent()
    {
        m_CubemapPtr = 0;
    }

    // -----------------------------------------------------------------------------

    CTexturePtr CSkyComponent::GetCubemapPtr() const
    {
        return m_CubemapPtr;
    }

    // -----------------------------------------------------------------------------

    Base::U64 CSkyComponent::GetTimeStamp() const
    {
        return m_TimeStamp;
    }
} // namespace Gfx