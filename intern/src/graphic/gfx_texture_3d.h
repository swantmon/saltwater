
#pragma once

#include "base/base_managed_pool.h"

#include "graphic/gfx_texture_base.h"

namespace Gfx
{
    class CTexture3D : public CTextureBase
    {
        public:

            enum
            {
                Dimension = Dim3D,
            };

        public:

            BPixels GetNumberOfPixelsU() const;
            BPixels GetNumberOfPixelsV() const;
            BPixels GetNumberOfPixelsW() const;

        protected:

            BPixels m_NumberOfPixels[3];

        protected:

            CTexture3D();
           ~CTexture3D();
    };
} // namespace Gfx

namespace Gfx
{
    typedef Base::CManagedPoolItemPtr<CTexture3D> CTexture3DPtr;
} // namespace Gfx
