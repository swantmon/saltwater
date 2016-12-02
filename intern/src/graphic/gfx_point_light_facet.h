#pragma once

#include "base/base_managed_pool.h"

#include "graphic/gfx_camera.h"
#include "graphic/gfx_texture_set.h"

namespace Gfx
{
    class CPointLightFacet : public Base::CManagedPoolItemBase
    {
    public:

        CPointLightFacet();
        ~CPointLightFacet();

    public:

        CTextureSetPtr GetTextureSMSet() const;
        CTextureSetPtr GetTextureRSMSet() const;

        CCameraPtr GetCamera() const;

        unsigned int GetShadowmapSize() const;

        Base::U64 GetTimeStamp() const;

    protected:

        CTextureSetPtr m_TextureSMPtr;
        CTextureSetPtr m_TextureRSMPtr;
        CCameraPtr     m_CameraPtr;
        unsigned int   m_ShadowmapSize;
        Base::U64      m_TimeStamp;
    };
} // namespace Gfx

namespace Gfx
{
    typedef Base::CManagedPoolItemPtr<CPointLightFacet> CPointLightFacetPtr;
} // namespace Gfx