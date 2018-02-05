#pragma once

#include "base/base_lib_glm.h"
#include "base/base_managed_pool.h"

#include "graphic/gfx_buffer.h"
#include "graphic/gfx_buffer_set.h"
#include "graphic/gfx_texture.h"

namespace Gfx
{
    class CAreaLightFacet : public Base::CManagedPoolItemBase
    {
    public:

        CAreaLightFacet();
        ~CAreaLightFacet();

    public:

        CBufferPtr GetPlaneIndexBuffer() const;
        CBufferPtr GetPlaneVertexBuffer() const;

        glm::vec4 GetDirectionX() const;
        glm::vec4 GetDirectionY() const;
        glm::vec4 GetPlane() const;
        float GetHalfWidth() const;
        float GetHalfHeight() const;

        CTexturePtr GetTexturePtr() const;
        CTexturePtr GetFilteredTexturePtr() const;

        bool HasTexture() const;

    protected:

        CBufferPtr    m_PlaneIndexBufferPtr;
        CBufferPtr    m_PlaneVertexBufferSetPtr;
        CTexturePtr m_TexturePtr;
        CTexturePtr m_FilteredTexturePtr;
        glm::vec4  m_DirectionX;
        glm::vec4  m_DirectionY;
        glm::vec4  m_Plane;
        float         m_HalfWidth;
        float         m_HalfHeight;
        Base::U64     m_TimeStamp;
    };
} // namespace Gfx

namespace Gfx
{
    typedef Base::CManagedPoolItemPtr<CAreaLightFacet> CAreaLightFacetPtr;
} // namespace Gfx