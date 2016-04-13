
#pragma once

#include "base/base_managed_pool.h"

#include "data/data_texture_base.h"
#include "data/data_texture_2d.h"

namespace Dt
{
    class CTextureCube : public CTextureBase
    {
        public:

            static const unsigned int s_NumberOfFaces = 6;

        public:

            enum
            {
                Dimension = Dim2D,
            };

            enum EFace
            {
                Front,
                Back,
                Left,
                Right,
                Top,
                Bottom,
            };

        public:

            CTexture2D* GetFace(const EFace _Face);

            BPixels GetNumberOfPixelsU() const;
            BPixels GetNumberOfPixelsV() const;

        protected:

            BPixels     m_NumberOfPixels[2];
            CTexture2D* m_pFaces[s_NumberOfFaces];

        protected:

            CTextureCube();
           ~CTextureCube();
    };
} // namespace Dt
