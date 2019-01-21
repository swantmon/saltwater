
#include "engine/engine_precompiled.h"

#include "engine/graphic/gfx_scissor_rect.h"

namespace Gfx
{
    CScissorRect::CScissorRect()
        : m_Rectangle({ 0, 0, 0, 0 })
    {
    }

    // -----------------------------------------------------------------------------

    CScissorRect::~CScissorRect()
    {
    }

    // -----------------------------------------------------------------------------

    void CScissorRect::SetTopLeftX(int _X)
    {
        m_Rectangle.m_TopLeftX = _X;
    }

    // -----------------------------------------------------------------------------

    int CScissorRect::GetTopLeftX() const
    {
        return m_Rectangle.m_TopLeftX;
    }

    // -----------------------------------------------------------------------------

    void CScissorRect::SetTopLeftY(int _Y)
    {
        m_Rectangle.m_TopLeftY = _Y;
    }

    // -----------------------------------------------------------------------------

    int CScissorRect::GetTopLeftY() const
    {
        return m_Rectangle.m_TopLeftY;
    }

    // -----------------------------------------------------------------------------

    void CScissorRect::SetWidth(int _Width)
    {
        m_Rectangle.m_Width = _Width;
    }

    // -----------------------------------------------------------------------------

    int CScissorRect::GetWidth() const
    {
        return m_Rectangle.m_Width;
    }

    // -----------------------------------------------------------------------------

    void CScissorRect::SetHeight(int _Height)
    {
        m_Rectangle.m_Height = _Height;
    }

    // -----------------------------------------------------------------------------

    int CScissorRect::GetHeight() const
    {
        return m_Rectangle.m_Height;
    }
} // namespace Gfx

