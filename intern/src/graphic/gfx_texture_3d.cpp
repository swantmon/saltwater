
#include "graphic/gfx_precompiled.h"

#include "graphic/gfx_texture_3d.h"

namespace Gfx
{
    CTexture3D::CTexture3D()
        : CTextureBase()
    {
    }

    // -----------------------------------------------------------------------------

    CTexture3D::~CTexture3D()
    {
    }

    // -----------------------------------------------------------------------------

    CTexture3D::BPixels CTexture3D::GetNumberOfPixelsU() const
    {
        return m_NumberOfPixels[0];
    }

    // -----------------------------------------------------------------------------

    CTexture3D::BPixels CTexture3D::GetNumberOfPixelsV() const
    {
        return m_NumberOfPixels[1];
    }

    // -----------------------------------------------------------------------------

    CTexture3D::BPixels CTexture3D::GetNumberOfPixelsW() const
    {
        return m_NumberOfPixels[2];
    }
} // namespace Gfx
