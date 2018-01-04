#pragma once

#include "base/base_managed_pool.h"

#include "graphic/gfx_texture.h"

namespace Gfx
{
    class CSkyFacet : public Base::CManagedPoolItemBase
    {
    public:

        CSkyFacet();
        ~CSkyFacet();

    public:

        CTexturePtr GetCubemapPtr() const;
        Base::U64 GetTimeStamp() const;

    protected:

        CTexturePtr m_CubemapPtr;
        Base::U64   m_TimeStamp;
    };
} // namespace Gfx

namespace Gfx
{
    typedef Base::CManagedPoolItemPtr<CSkyFacet> CSkyFacetPtr;
} // namespace Gfx