
#include "engine/engine_precompiled.h"

#include "engine/graphic/gfx_view_port.h"

namespace Gfx
{
    CViewPort::CViewPort()
    {
    }

    // -----------------------------------------------------------------------------

    CViewPort::~CViewPort()
    {
    }

    // -----------------------------------------------------------------------------

    float CViewPort::GetWidth() const
    {
        return m_Port.m_Width;
    }

    // -----------------------------------------------------------------------------

    float CViewPort::GetHeight() const
    {
        return m_Port.m_Height;
    }

    // -----------------------------------------------------------------------------

    float CViewPort::GetMinDepth() const
    {
        return m_Port.m_MinDepth;
    }

    // -----------------------------------------------------------------------------

    float CViewPort::GetMaxDepth() const
    {
        return m_Port.m_MaxDepth;
    }

    // -----------------------------------------------------------------------------

    float CViewPort::GetTopLeftX() const
    {
        return m_Port.m_TopLeftX;
    }

    // -----------------------------------------------------------------------------

    float CViewPort::GetTopLeftY() const
    {
        return m_Port.m_TopLeftY;
    }
} // namespace Gfx

