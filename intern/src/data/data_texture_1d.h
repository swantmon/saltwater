
#pragma once

#include "base/base_managed_pool.h"

#include "data/data_texture_base.h"

namespace Dt
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
} // namespace Dt
