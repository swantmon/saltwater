
#include "data/data_precompiled.h"

#include "data/data_texture_1d.h"

namespace Dt
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
} // namespace Dt
