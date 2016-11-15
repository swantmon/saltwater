//
//  gfx_material.h
//  graphic
//
//  Created by Tobias Schwandt on 03/11/14.
//  Copyright (c) 2014 TU Ilmenau. All rights reserved.
//

#pragma once

#include "base/base_managed_pool.h"
#include "base/base_vector3.h"
#include "base/base_vector4.h"

#include "graphic/gfx_buffer.h"
#include "graphic/gfx_sampler_set.h"
#include "graphic/gfx_shader.h"
#include "graphic/gfx_texture_set.h"

namespace Gfx
{
    class CMaterial : public Base::CManagedPoolItemBase
    {
    public:
        
#pragma warning(disable:4201)
        struct SMaterialKey
        {
            static const unsigned int s_NumberOfTextures = 6;
            
            typedef unsigned char BMaterialID;
            
            union
            {
                struct
                {
                    BMaterialID m_HasDiffuseTex         :  1;        //< Defines either a diffuse texture is set
                    BMaterialID m_HasNormalTex          :  1;        //< Defines either a normal texture is set
                    BMaterialID m_HasRoughnessTex       :  1;        //< Defines either a roughness texture is set
                    BMaterialID m_HasMetallicTex        :  1;        //< Defines either a metallic texture is set
                    BMaterialID m_HasAOTex              :  1;        //< Defines either a ambient occlusion texture is set
                    BMaterialID m_HasBumpTex            :  1;        //< Defines either a height/bump/displacement texture is set
                    BMaterialID m_Padding               :  1;
                };
                BMaterialID m_Key;
            };
        };
#pragma warning(default:4201)
        
        struct SMaterialAttributes
        {
            Base::Float4  m_TilingOffset;
            Base::Float3  m_Color;
            float         m_Roughness;
            float         m_Reflectance;
            float         m_MetalMask;
        };
        
    public:

        CShaderPtr GetShaderHS() const;
        CShaderPtr GetShaderDS() const;
        CShaderPtr GetShaderGS() const;
        CShaderPtr GetShaderPS() const;

    public:

        CSamplerSetPtr GetSamplerSetHS() const;
        CSamplerSetPtr GetSamplerSetDS() const;
        CSamplerSetPtr GetSamplerSetGS() const;
        CSamplerSetPtr GetSamplerSetPS() const;

    public:

        CTextureSetPtr GetTextureSetHS() const;
        CTextureSetPtr GetTextureSetDS() const;
        CTextureSetPtr GetTextureSetGS() const;
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
        
        CShaderPtr  m_ShaderPtrs[CShader::NumberOfTypes];

        CSamplerSetPtr m_SamplerSetPtrs[CShader::NumberOfTypes];

        CTextureSetPtr m_TextureSetPtrs[CShader::NumberOfTypes];
        
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
