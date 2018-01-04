
#pragma once

#include "graphic/gfx_native_types.h"
#include "graphic/gfx_texture.h"

namespace Gfx
{
    class CNativeTexture : public CTexture
    {
    public:
        
        GLuint m_NativeTexture;
        GLenum m_NativeUsage;
        GLenum m_NativeInternalFormat;
        GLenum m_NativeBinding;
    };
} // namespace Gfx
