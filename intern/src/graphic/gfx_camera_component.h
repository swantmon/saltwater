
#pragma once

#include "graphic/gfx_component.h"
#include "graphic/gfx_texture.h"
#include "graphic/gfx_texture_set.h"

namespace Gfx
{
    class CCameraComponent : public CComponent<CCameraComponent>
    {
    public:

        void SetBackgroundTexture2D(CTexturePtr _Texture2DPtr);
        CTexturePtr GetBackgroundTexture2D();

        void SetBackgroundTextureSet(CTextureSetPtr _TextureSetPtr);
        CTextureSetPtr GetBackgroundTextureSet();

        void SetTimeStamp(Base::U64 _TimeStamp);
        Base::U64 GetTimeStamp();

    public:

        CCameraComponent();
        ~CCameraComponent();

    protected:

        CTexturePtr    m_BackgroundTexture2DPtr;
        CTextureSetPtr m_BackgroundTextureSetPtr;
        Base::U64      m_TimeStamp;
    };
} // namespace Gfx