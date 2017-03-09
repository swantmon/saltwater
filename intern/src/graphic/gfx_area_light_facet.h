#pragma once

#include "base/base_managed_pool.h"
#include "base/base_vector4.h"

#include "graphic/gfx_buffer.h"
#include "graphic/gfx_buffer_set.h"
#include "graphic/gfx_texture_2d.h"

namespace Gfx
{
    class CAreaLightFacet : public Base::CManagedPoolItemBase
    {
    public:

        CAreaLightFacet();
        ~CAreaLightFacet();

    public:

        CBufferPtr GetPlaneIndexBuffer() const;
        CBufferSetPtr GetPlaneVertexBuffer() const;

        Base::Float4 GetDirectionX() const;
        Base::Float4 GetDirectionY() const;
        Base::Float4 GetPlane() const;
        float GetHalfWidth() const;
        float GetHalfHeight() const;

        CTexture2DPtr GetTexturePtr() const;
        CTexture2DPtr GetFilteredTexturePtr() const;

        bool HasTexture() const;

    protected:

        CBufferPtr    m_PlaneIndexBufferPtr;
        CBufferSetPtr m_PlaneVertexBufferSetPtr;
        CTexture2DPtr m_TexturePtr;
        CTexture2DPtr m_FilteredTexturePtr;
        Base::Float4  m_DirectionX;
        Base::Float4  m_DirectionY;
        Base::Float4  m_Plane;
        float         m_HalfWidth;
        float         m_HalfHeight;
        Base::U64     m_TimeStamp;
    };
} // namespace Gfx

namespace Gfx
{
    typedef Base::CManagedPoolItemPtr<CAreaLightFacet> CAreaLightFacetPtr;
} // namespace Gfx