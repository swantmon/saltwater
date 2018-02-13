
#pragma once

#include "base/base_typedef.h"

#include "graphic/gfx_camera.h"
#include "graphic/gfx_component.h"
#include "graphic/gfx_texture_set.h"

namespace Gfx
{
    class CSunComponent : public CComponent<CSunComponent>
    {
    public:

        CSunComponent();
        ~CSunComponent();

    public:

        CTexturePtr GetShadowMapPtr() const;

        CCameraPtr GetCamera() const;

        Base::U64 GetTimeStamp() const;

    protected:

        CTexturePtr m_TextureSMPtr;
        CCameraPtr  m_CameraPtr;
        Base::U64   m_TimeStamp;
    };
} // namespace Gfx