
#include "graphic/gfx_precompiled.h"

#include "graphic/gfx_texture_set.h"

#include <assert.h>

namespace Gfx
{
    CTextureSet::CTextureSet()
        : m_ID              (s_InvalidID)
        , m_NumberOfTextures(0)
    {
    }

    // -----------------------------------------------------------------------------

    CTextureSet::~CTextureSet()
    {
    }

    // -----------------------------------------------------------------------------

    unsigned int CTextureSet::GetID() const
    {
        return m_ID;
    }

    // -----------------------------------------------------------------------------

    unsigned int CTextureSet::GetNumberOfTextures() const
    {
        return m_NumberOfTextures;
    }

    // -----------------------------------------------------------------------------

    CTexturePtr CTextureSet::GetTexture(unsigned int _Index)
    {
        assert(_Index < s_MaxNumberOfTextures);

        return m_TexturePtrs[_Index];
    }
} // namespace Gfx
