#pragma once

#include "base/base_managed_pool.h"

#include "graphic/gfx_texture.h"
#include "graphic/gfx_texture_set.h"

namespace Gfx
{
    class CSkyFacet : public Base::CManagedPoolItemBase
    {
    public:

        CSkyFacet();
        ~CSkyFacet();

    public:

        CTexturePtr GetCubemapPtr() const;

        // TODO: Remove this method because only the other one is needed!
        CTextureSetPtr GetCubemapSetPtr() const;

        Base::U64 GetTimeStamp() const;

    protected:

        CTexturePtr  m_CubemapPtr;
        CTextureSetPtr m_CubemapSetPtr;
        Base::U64      m_TimeStamp;
    };
} // namespace Gfx

namespace Gfx
{
    typedef Base::CManagedPoolItemPtr<CSkyFacet> CSkyFacetPtr;
} // namespace Gfx