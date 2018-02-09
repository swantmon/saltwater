
#pragma once

namespace Gfx
{
    class CGraphicsInfo
    {
    public:

        enum EGraphicAPI
        {
            OpenGL,
            OpenGLES,
            UndefinedAPI = -1
        };

        EGraphicAPI m_GraphicsAPI;
        int         m_MajorVersion;
        int         m_MinorVersion;
    };
} // namespace Gfx