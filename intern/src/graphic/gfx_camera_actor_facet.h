
#pragma once

#include "graphic/gfx_texture_2d.h"
#include "graphic/gfx_texture_set.h"

namespace Gfx
{
    class CCameraActorFacet
    {
    public:

        void SetBackgroundTexture2D(CTexture2DPtr _Texture2DPtr);
        CTexture2DPtr GetBackgroundTexture2D();

        void SetBackgroundTextureSet(CTextureSetPtr _TextureSetPtr);
        CTextureSetPtr GetBackgroundTextureSet();

        void SetTimeStamp(Base::U64 _TimeStamp);
        Base::U64 GetTimeStamp();

    public:

        CCameraActorFacet();
        ~CCameraActorFacet();

    protected:

        CTexture2DPtr     m_BackgroundTexture2DPtr;
        CTextureSetPtr    m_BackgroundTextureSetPtr;
        Base::U64         m_TimeStamp;
    };
} // namespace Gfx