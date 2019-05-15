
#include "engine/engine_precompiled.h"

#include "engine/graphic/gfx_sky.h"

namespace Gfx
{
    CSky::CSky()
        : m_CubemapPtr     ()
        , m_TimeStamp      (static_cast<Base::U64>(-1))
        , m_InputTexturePtr()
    {

    }

    // -----------------------------------------------------------------------------

    CSky::~CSky()
    {
        m_CubemapPtr = 0;
    }

    // -----------------------------------------------------------------------------

    CTexturePtr CSky::GetCubemapPtr() const
    {
        return m_CubemapPtr;
    }

    // -----------------------------------------------------------------------------

    Base::U64 CSky::GetTimeStamp() const
    {
        return m_TimeStamp;
    }

    // -----------------------------------------------------------------------------

    void CSky::SetInputTexture(Gfx::CTexturePtr _InputTexturePtr)
    {
        m_InputTexturePtr = _InputTexturePtr;
    }
} // namespace Gfx