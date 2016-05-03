
#include "graphic/gfx_precompiled.h"

#include "graphic/gfx_texture_2d.h"

namespace Gfx
{
    CTexture2D::CTexture2D()
        : CTextureBase()
    {
    }

    // -----------------------------------------------------------------------------

    CTexture2D::~CTexture2D()
    {
    }

    // -----------------------------------------------------------------------------

    CTexture2D::BPixels CTexture2D::GetNumberOfPixelsU() const
    {
        return m_NumberOfPixels[0];
    }

    // -----------------------------------------------------------------------------

    CTexture2D::BPixels CTexture2D::GetNumberOfPixelsV() const
    {
        return m_NumberOfPixels[1];
    }
} // namespace Gfx

