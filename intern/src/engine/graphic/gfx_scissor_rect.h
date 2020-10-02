
#pragma once

#include "engine/engine_config.h"

namespace Gfx
{
    class ENGINE_API CScissorRect
    {
    public:

        CScissorRect();
        ~CScissorRect();

    public:

        void SetTopLeftX(int _X);
        int GetTopLeftX() const;

        void SetTopLeftY(int _Y);
        int GetTopLeftY() const;

        void SetWidth(int _Width);
        int GetWidth() const;

        void SetHeight(int _Width);
        int GetHeight() const;

    public:

        struct SRectangle
        {
            int m_TopLeftX;
            int m_TopLeftY;
            int m_Width;
            int m_Height;
        };

    protected:

        SRectangle m_Rectangle;
    };
} // namespace Gfx
