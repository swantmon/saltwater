
#pragma once

#include "graphic/gfx_native_types.h"
#include "graphic/gfx_texture.h"

namespace Gfx
{
    class CNativeTexture : public CTexture
    {
    public:
        
        unsigned int GetNativeHandle() const override
        {
            return m_NativeTexture;
        }

        // -----------------------------------------------------------------------------

        GLenum GetNativeUsage()
        {
            return m_NativeUsage;
        }

        // -----------------------------------------------------------------------------

        GLenum GetNativeInternalFormat()
        {
            return m_NativeInternalFormat;
        }

        // -----------------------------------------------------------------------------

        GLenum GetNativeBinding()
        {
            return m_NativeBinding;
        }

    protected:

        GLuint m_NativeTexture;
        GLenum m_NativeUsage;
        GLenum m_NativeInternalFormat;
        GLenum m_NativeBinding;
    };
} // namespace Gfx
