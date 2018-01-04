#pragma once

#include "base/base_managed_pool.h"

#include "graphic/gfx_texture.h"

namespace Gfx
{
    class CLightProbeFacet : public Base::CManagedPoolItemBase
    {
    public:

        CLightProbeFacet();
        ~CLightProbeFacet();

    public:

        CTexturePtr GetDiffusePtr() const;

        CTexturePtr GetSpecularPtr() const;

        CTexturePtr GetDepthPtr() const;

        Base::U64 GetTimeStamp() const;

    protected:

        CTexturePtr m_DiffusePtr;
        CTexturePtr m_SpecularPtr;
        CTexturePtr m_DepthPtr;
        Base::U64   m_TimeStamp;
    };
} // namespace Gfx

namespace Gfx
{
    typedef Base::CManagedPoolItemPtr<CLightProbeFacet> CLightProbeFacetPtr;
} // namespace Gfx