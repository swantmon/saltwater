
#pragma once

#include "base/base_include_glm.h"
#include "base/base_managed_pool.h"

#include "engine/graphic/gfx_buffer.h"
#include "engine/graphic/gfx_sampler_set.h"
#include "engine/graphic/gfx_shader.h"
#include "engine/graphic/gfx_texture_set.h"

namespace Gfx
{
    class ENGINE_API CMaterial : public Base::CManagedPoolItemBase
    {
    public:

        typedef unsigned int BHash;

    public:

#pragma warning(push)
#pragma warning(disable:4201)
        struct SMaterialKey
        {
            static const unsigned int s_NumberOfTextures = 7;

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
                    BMaterialID m_HasAlphaTex     : 1;        //< Defines either a alpha texture is set
                };
                BMaterialID m_Key;
            };
        };
#pragma warning(pop)

        struct SMaterialAttributes
        {
            glm::vec4 m_TilingOffset;
            glm::vec4 m_Color;
            float     m_Roughness;
            float     m_Reflectance;
            float     m_MetalMask;
            float     m_Displacement;
        };

        struct SRefractionAttributes
        {
            float m_IndexOfRefraction;
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

        const SRefractionAttributes& GetMaterialRefractionAttributes() const;

        const SMaterialKey& GetKey() const;

        bool HasAlpha() const;
        bool HasBump() const;
        bool HasRefraction() const;

        BHash GetHash() const;

    protected:

        CMaterial();
        ~CMaterial();

    protected:

        CShaderPtr m_ShaderPtrs[CShader::NumberOfTypes];

        CShaderPtr m_ForwardShaderPSPtr;

        CSamplerSetPtr m_SamplerSetPtr;

        CTextureSetPtr m_TextureSetPtr;

        SMaterialAttributes m_MaterialAttributes;

        SRefractionAttributes m_MaterialRefractionAttributes;

        SMaterialKey m_MaterialKey;

        BHash m_Hash;
    };
} // namespace Gfx

namespace Gfx
{
    typedef Base::CManagedPoolItemPtr<CMaterial> CMaterialPtr;
} // namespace Gfx
