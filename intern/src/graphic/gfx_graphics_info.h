
#pragma once

namespace Gfx
{
    class CGraphicsInfo
    {
    public:

        enum EGraphicAPI
        {
            OpenGL       = 0,
            OpenGLES     = 1,
            UndefinedAPI = -1
        };

        EGraphicAPI m_GraphicsAPI;
        int         m_MajorVersion;
        int         m_MinorVersion;
    };
} // namespace Gfx