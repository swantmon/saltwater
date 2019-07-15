#pragma once

#include "engine/graphic/gfx_texture.h"

namespace Gfx
{
    class ENGINE_API CSky : public Base::CManagedPoolItemBase
    {
    public:

        CSky();
        ~CSky();

    public:

        CTexturePtr GetCubemapPtr() const;
        Base::U64 GetTimeStamp() const;

    public:

        void SetInputTexture(Gfx::CTexturePtr _InputTexturePtr);

    protected:

        CTexturePtr m_CubemapPtr;
        CTexturePtr m_InputTexturePtr;
        Base::U64   m_TimeStamp;
    };
} // namespace Gfx

namespace Gfx
{
    using CSkyPtr = Base::CManagedPoolItemPtr<CSky>;
} // namespace Gfx