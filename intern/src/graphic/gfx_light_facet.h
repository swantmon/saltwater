
#pragma once

#include "base/base_typedef.h"

#include "graphic/gfx_render_context.h"
#include "graphic/gfx_texture_2d.h"
#include "graphic/gfx_texture_set.h"

namespace Gfx
{
    class CPointLightFacet
    {
    public:

        void SetRenderContext(CRenderContextPtr _RenderContextPtr);
        CRenderContextPtr GetRenderContext();

        void SetTextureSMSet(CTextureSetPtr _TextureSMPtr);
        CTextureSetPtr GetTextureSMSet();

        void SetTimeStamp(Base::U64 _TimeStamp);
        Base::U64 GetTimeStamp();

    public:

        CPointLightFacet();
        ~CPointLightFacet();

    protected:

        CRenderContextPtr m_RenderContextPtr;
        CTextureSetPtr    m_TextureSMPtr;
        Base::U64         m_TimeStamp;
    };
} // namespace Gfx

namespace Gfx
{
    class CSunLightFacet
    {
    public:

        void SetRenderContext(CRenderContextPtr _RenderContextPtr);
        CRenderContextPtr GetRenderContext();

        void SetTextureSMSet(CTextureSetPtr _TextureSMPtr);
        CTextureSetPtr GetTextureSMSet();

        void SetTimeStamp(Base::U64 _TimeStamp);
        Base::U64 GetTimeStamp();

    public:

        CSunLightFacet();
        ~CSunLightFacet();

    protected:

        CRenderContextPtr m_RenderContextPtr;
        CTextureSetPtr    m_TextureSMPtr;
        Base::U64         m_TimeStamp;
    };
} // namespace Gfx