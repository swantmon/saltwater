
#pragma once

#include "base/base_include_glm.h"
#include "base/base_managed_pool.h"

#include "graphic/gfx_buffer.h"
#include "graphic/gfx_sampler_set.h"
#include "graphic/gfx_shader.h"
#include "graphic/gfx_texture_set.h"

namespace Gfx
{
    class CMaterial : public Base::CManagedPoolItemBase
    {
    public:

#pragma warning(push)
#pragma warning(disable:4201)
        struct SMaterialKey
        {
            static const unsigned int s_NumberOfTextures = 6;

            typedef unsigned char BMaterialID;

            union
            {
                struct
                {
                    BMaterialID m_HasDiffuseTex   : 1;        //< Defines either a diffuse texture is set
                    BMaterialID m_HasNormalTex    : 1;        //< Defines either a normal texture is set
                    BMaterialID m_HasRoughnessTex : 1;        //< Defines either a roughness texture is set
                    BMaterialID m_HasMetallicTex  : 1;        //< Defines either a metallic texture is set
                    BMaterialID m_HasAOTex        : 1;        //< Defines either a ambient occlusion texture is set
                    BMaterialID m_HasBumpTex      : 1;        //< Defines either a bump/displacement texture is set
                    BMaterialID m_Padding         : 1;
                };
                BMaterialID m_Key;
            };
        };
#pragma warning(pop)

        struct SMaterialAttributes
        {
            glm::vec4 m_TilingOffset;
            glm::vec3 m_Color;
            float     m_Roughness;
            float     m_Reflectance;
            float     m_MetalMask;
            float     m_Displacement;
        };

    public:

        CShaderPtr GetShaderGS() const;
        CShaderPtr GetShaderDS() const;
        CShaderPtr GetShaderHS() const;
        CShaderPtr GetShaderPS() const;
        CShaderPtr GetForwardShaderPS() const;

    public:

        CSamplerSetPtr GetSamplerSetPS() const;

    public:

        CTextureSetPtr GetTextureSetPS() const;

    public:

        const SMaterialAttributes& GetMaterialAttributes() const;

        const SMaterialKey& GetKey() const;

        bool GetHasAlpha() const;
        bool GetHasBump() const;

        unsigned int GetHash() const;

    protected:

        CMaterial();
        ~CMaterial();

    protected:

        CShaderPtr m_ShaderPtrs[CShader::NumberOfTypes];

        CShaderPtr m_ForwardShaderPSPtr;

        CSamplerSetPtr m_SamplerSetPtr;

        CTextureSetPtr m_TextureSetPtr;

        SMaterialAttributes m_MaterialAttributes;
        SMaterialKey        m_MaterialKey;

        bool m_HasAlpha;
        bool m_HasBump;

        unsigned int m_Hash;
    };
} // namespace Gfx

namespace Gfx
{
    typedef Base::CManagedPoolItemPtr<CMaterial> CMaterialPtr;
} // namespace Gfx
