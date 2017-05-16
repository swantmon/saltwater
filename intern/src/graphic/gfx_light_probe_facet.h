#pragma once

#include "base/base_managed_pool.h"

#include "graphic/gfx_texture_2d.h"
#include "graphic/gfx_texture_set.h"

namespace Gfx
{
    class CLightProbeFacet : public Base::CManagedPoolItemBase
    {
    public:

        CLightProbeFacet();
        ~CLightProbeFacet();

    public:

        CTexture2DPtr GetDiffusePtr() const;

        CTexture2DPtr GetSpecularPtr() const;

        CTexture2DPtr GetDepthPtr() const;

        Base::U64 GetTimeStamp() const;

    protected:

        CTexture2DPtr  m_DiffusePtr;
        CTexture2DPtr  m_SpecularPtr;
        CTexture2DPtr  m_DepthPtr;
        Base::U64      m_TimeStamp;
    };
} // namespace Gfx

namespace Gfx
{
    typedef Base::CManagedPoolItemPtr<CLightProbeFacet> CLightProbeFacetPtr;
} // namespace Gfx