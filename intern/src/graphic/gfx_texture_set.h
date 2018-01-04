
#pragma once

#include "base/base_managed_pool.h"

#include "graphic/gfx_texture.h"

namespace Gfx
{
    class CTextureSet : public Base::CManagedPoolItemBase
    {
        public:

            static const unsigned int s_MaxNumberOfTextures = 16;
            static const unsigned int s_InvalidID           = static_cast<unsigned int>(-1);

        public:

            unsigned int GetID() const;

        public:

            unsigned int GetNumberOfTextures() const;

            CTexturePtr GetTexture(unsigned int _Index);

        protected:

            unsigned int              m_ID;
            unsigned int              m_NumberOfTextures;
            CTexturePtr           m_TexturePtrs[s_MaxNumberOfTextures];

        protected:

            CTextureSet();
           ~CTextureSet();
    };
} // namespace Gfx

namespace Gfx
{
    typedef Base::CManagedPoolItemPtr<CTextureSet> CTextureSetPtr;
} // namespace Gfx
