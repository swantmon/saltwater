
#include "graphic/gfx_precompiled.h"

#include "graphic/gfx_texture_1d.h"

namespace Gfx
{
    CTexture1D::CTexture1D()
        : CTextureBase()
    {
    }

    // -----------------------------------------------------------------------------

    CTexture1D::~CTexture1D()
    {
    }

    // -----------------------------------------------------------------------------

    CTexture1D::BPixels CTexture1D::GetNumberOfPixelsU() const
    {
        return m_NumberOfPixels[0];
    }
} // namespace Gfx
