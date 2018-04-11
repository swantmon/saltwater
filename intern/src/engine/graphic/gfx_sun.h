
#pragma once

#include "base/base_typedef.h"

#include "engine/graphic/gfx_camera.h"
#include "engine/graphic/gfx_texture_set.h"

namespace Gfx
{
    class CSun : public Base::CManagedPoolItemBase
    {
    public:

        CSun();
        ~CSun();

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

namespace Gfx
{
    typedef Base::CManagedPoolItemPtr<CSun> CSunPtr;
} // namespace Gfx