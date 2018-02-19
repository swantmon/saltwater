#pragma once

#include "base/base_include_glm.h"

#include "graphic/gfx_component.h"
#include "graphic/gfx_buffer.h"
#include "graphic/gfx_buffer_set.h"
#include "graphic/gfx_texture.h"

namespace Gfx
{
    class CAreaLightComponent : public CComponent<CAreaLightComponent>
    {
    public:

        CAreaLightComponent();
        ~CAreaLightComponent();

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

        CBufferPtr  m_PlaneIndexBufferPtr;
        CBufferPtr  m_PlaneVertexBufferSetPtr;
        CTexturePtr m_TexturePtr;
        CTexturePtr m_FilteredTexturePtr;
        glm::vec4   m_DirectionX;
        glm::vec4   m_DirectionY;
        glm::vec4   m_Plane;
        float       m_HalfWidth;
        float       m_HalfHeight;
        Base::U64   m_TimeStamp;
    };
} // namespace Gfx