#pragma once

#include "graphic/gfx_texture.h"

namespace Gfx
{
    class CSky : public Base::CManagedPoolItemBase
    {
    public:

        CSky();
        ~CSky();

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
    typedef Base::CManagedPoolItemPtr<CSky> CSkyPtr;
} // namespace Gfx