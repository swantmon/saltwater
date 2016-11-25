
#pragma once

#include "base/base_typedef.h"

#include "graphic/gfx_camera.h"
#include "graphic/gfx_texture_set.h"

namespace Gfx
{
    class CSunFacet : public Base::CManagedPoolItemBase
    {
    public:

        CSunFacet();
        ~CSunFacet();

    public:

        CTextureSetPtr GetTextureSMSet() const;

        CCameraPtr GetCamera() const;

        Base::U64 GetTimeStamp() const;

    protected:

        CTextureSetPtr m_TextureSMPtr;
        CCameraPtr     m_CameraPtr;
        Base::U64      m_TimeStamp;
    };
} // namespace Gfx

namespace Gfx
{
    typedef Base::CManagedPoolItemPtr<CSunFacet> CSunFacetPtr;
} // namespace Gfx