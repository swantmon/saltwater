
#pragma once

#include "base/base_managed_pool.h"

#include "data/data_texture_base.h"

namespace Dt
{
    class CTexture2D : public CTextureBase
    {
        public:

            enum
            {
                Dimension = Dim2D,
            };

        public:

            BPixels GetNumberOfPixelsU() const;
            BPixels GetNumberOfPixelsV() const;

        protected:

            BPixels m_NumberOfPixels[2];

        protected:

            CTexture2D();
           ~CTexture2D();
    };
} // namespace Dt