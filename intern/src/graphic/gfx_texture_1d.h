
#pragma once

#include "base/base_managed_pool.h"

#include "graphic/gfx_texture_base.h"

namespace Gfx
{
    class CTexture1D : public CTextureBase
    {
        public:

            enum
            {
                Dimension = Dim1D,
            };

        public:

            BPixels GetNumberOfPixelsU() const;

        protected:

            BPixels m_NumberOfPixels[1];

        protected:

            CTexture1D();
           ~CTexture1D();
    };
} // namespace Gfx

namespace Gfx
{
    typedef Base::CManagedPoolItemPtr<CTexture1D> CTexture1DPtr;
} // namespace Gfx
