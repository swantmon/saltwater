#pragma once

#include "base/base_managed_pool.h"

namespace Gfx
{
    class CAreaLightFacet : public Base::CManagedPoolItemBase
    {
    public:

        CAreaLightFacet();
        ~CAreaLightFacet();

    protected:

        Base::U64 m_TimeStamp;
    };
} // namespace Gfx

namespace Gfx
{
    typedef Base::CManagedPoolItemPtr<CAreaLightFacet> CAreaLightFacetPtr;
} // namespace Gfx