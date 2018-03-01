#pragma once

#include "graphic/gfx_texture.h"

namespace Gfx
{
    class CLightProbe : public Base::CManagedPoolItemBase
    {
    public:

        CLightProbe();
        ~CLightProbe();

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
    typedef Base::CManagedPoolItemPtr<CLightProbe> CLightProbePtr;
} // namespace Gfx