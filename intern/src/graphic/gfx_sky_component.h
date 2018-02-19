#pragma once

#include "graphic/gfx_component.h"
#include "graphic/gfx_texture.h"

namespace Gfx
{
    class CSkyComponent : public CComponent<CSkyComponent>
    {
    public:

        CSkyComponent();
        ~CSkyComponent();

    public:

        CTexturePtr GetCubemapPtr() const;
        Base::U64 GetTimeStamp() const;

    protected:

        CTexturePtr m_CubemapPtr;
        Base::U64   m_TimeStamp;
    };
} // namespace Gfx