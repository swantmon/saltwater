
#include "data/data_texture_cube.h"

namespace Dt
{
    CTextureCube::CTextureCube()
        : CTextureBase()
    {
    }

    // -----------------------------------------------------------------------------

    CTextureCube::~CTextureCube()
    {
    }

    // -----------------------------------------------------------------------------

    CTexture2D* CTextureCube::GetFace(const EFace _Face)
    {
        return m_pFaces[_Face];
    }

    // -----------------------------------------------------------------------------

    CTextureCube::BPixels CTextureCube::GetNumberOfPixelsU() const
    {
        return m_NumberOfPixels[0];
    }

    // -----------------------------------------------------------------------------

    CTextureCube::BPixels CTextureCube::GetNumberOfPixelsV() const
    {
        return m_NumberOfPixels[1];
    }
} // namespace Dt

