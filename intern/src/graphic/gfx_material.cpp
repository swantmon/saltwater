
#include "graphic/gfx_precompiled.h"

#include "base/base_memory.h"

#include "graphic/gfx_material.h"

namespace Gfx
{
    CMaterial::CMaterial()
        : m_HasAlpha(false)
        , m_HasBump (false)
    {
        m_MaterialAttributes.m_Color[0]        = 1.0f;
        m_MaterialAttributes.m_Color[1]        = 1.0f;
        m_MaterialAttributes.m_Color[2]        = 1.0f;
        m_MaterialAttributes.m_Roughness       = 0.0f;
        m_MaterialAttributes.m_Reflectance     = 0.0f;
        m_MaterialAttributes.m_MetalMask       = 0.0f;
        m_MaterialAttributes.m_TilingOffset[0] = 1.0f;
        m_MaterialAttributes.m_TilingOffset[1] = 1.0f;
        m_MaterialAttributes.m_TilingOffset[2] = 0.0f;
        m_MaterialAttributes.m_TilingOffset[3] = 0.0f;
        
        m_MaterialKey.m_Key = 0;

        m_ShaderPtrs[CShader::Vertex]   = 0;
        m_ShaderPtrs[CShader::Hull]     = 0;
        m_ShaderPtrs[CShader::Domain]   = 0;
        m_ShaderPtrs[CShader::Geometry] = 0;
        m_ShaderPtrs[CShader::Pixel]    = 0;

        m_SamplerSetPtrs[CShader::Vertex]   = 0;
        m_SamplerSetPtrs[CShader::Hull]     = 0;
        m_SamplerSetPtrs[CShader::Domain]   = 0;
        m_SamplerSetPtrs[CShader::Geometry] = 0;
        m_SamplerSetPtrs[CShader::Pixel]    = 0;

        m_TextureSetPtrs[CShader::Vertex]   = 0;
        m_TextureSetPtrs[CShader::Hull]     = 0;
        m_TextureSetPtrs[CShader::Domain]   = 0;
        m_TextureSetPtrs[CShader::Geometry] = 0;
        m_TextureSetPtrs[CShader::Pixel]    = 0;
    }
    
    // -----------------------------------------------------------------------------
    
    CMaterial::~CMaterial()
    {
        m_ShaderPtrs[CShader::Vertex]   = 0;
        m_ShaderPtrs[CShader::Hull]     = 0;
        m_ShaderPtrs[CShader::Domain]   = 0;
        m_ShaderPtrs[CShader::Geometry] = 0;
        m_ShaderPtrs[CShader::Pixel]    = 0;

        m_SamplerSetPtrs[CShader::Vertex]   = 0;
        m_SamplerSetPtrs[CShader::Hull]     = 0;
        m_SamplerSetPtrs[CShader::Domain]   = 0;
        m_SamplerSetPtrs[CShader::Geometry] = 0;
        m_SamplerSetPtrs[CShader::Pixel]    = 0;

        m_TextureSetPtrs[CShader::Vertex]   = 0;
        m_TextureSetPtrs[CShader::Hull]     = 0;
        m_TextureSetPtrs[CShader::Domain]   = 0;
        m_TextureSetPtrs[CShader::Geometry] = 0;
        m_TextureSetPtrs[CShader::Pixel]    = 0;
    }

    // -----------------------------------------------------------------------------

    CShaderPtr CMaterial::GetShaderVS() const
    {
        return m_ShaderPtrs[CShader::Vertex];
    }

    // -----------------------------------------------------------------------------
    
    CShaderPtr CMaterial::GetShaderHS() const
    {
        return m_ShaderPtrs[CShader::Hull];
    }
    
    // -----------------------------------------------------------------------------
    
    CShaderPtr CMaterial::GetShaderDS() const
    {
        return m_ShaderPtrs[CShader::Domain];
    }
    
    // -----------------------------------------------------------------------------
    
    CShaderPtr CMaterial::GetShaderGS() const
    {
        return m_ShaderPtrs[CShader::Geometry];
    }

    // -----------------------------------------------------------------------------

    CShaderPtr CMaterial::GetShaderPS() const
    {
        return m_ShaderPtrs[CShader::Pixel];
    }

    // -----------------------------------------------------------------------------

    CSamplerSetPtr CMaterial::GetSamplerSetVS() const
    {
        return m_SamplerSetPtrs[CShader::Vertex];
    }

    // -----------------------------------------------------------------------------

    CSamplerSetPtr CMaterial::GetSamplerSetHS() const
    {
        return m_SamplerSetPtrs[CShader::Hull];
    }

    // -----------------------------------------------------------------------------

    CSamplerSetPtr CMaterial::GetSamplerSetDS() const
    {
        return m_SamplerSetPtrs[CShader::Domain];
    }

    // -----------------------------------------------------------------------------

    CSamplerSetPtr CMaterial::GetSamplerSetGS() const
    {
        return m_SamplerSetPtrs[CShader::Geometry];
    }

    // -----------------------------------------------------------------------------

    CSamplerSetPtr CMaterial::GetSamplerSetPS() const
    {
        return m_SamplerSetPtrs[CShader::Pixel];
    }
    
    // -----------------------------------------------------------------------------
    
    CTextureSetPtr CMaterial::GetTextureSetVS() const
    {
        return m_TextureSetPtrs[CShader::Vertex];
    }

    // -----------------------------------------------------------------------------

    CTextureSetPtr CMaterial::GetTextureSetHS() const
    {
        return m_TextureSetPtrs[CShader::Hull];
    }

    // -----------------------------------------------------------------------------

    CTextureSetPtr CMaterial::GetTextureSetDS() const
    {
        return m_TextureSetPtrs[CShader::Domain];
    }

    // -----------------------------------------------------------------------------

    CTextureSetPtr CMaterial::GetTextureSetGS() const
    {
        return m_TextureSetPtrs[CShader::Geometry];
    }

    // -----------------------------------------------------------------------------

    CTextureSetPtr CMaterial::GetTextureSetPS() const
    {
        return m_TextureSetPtrs[CShader::Pixel];
    }
    
    // -----------------------------------------------------------------------------
    
    const CMaterial::SMaterialAttributes& CMaterial::GetMaterialAttributes() const
    {
        return m_MaterialAttributes;
    }
    
    // -----------------------------------------------------------------------------
    
    const CMaterial::SMaterialKey& CMaterial::GetKey() const
    {
        return m_MaterialKey;
    }
    
    // -----------------------------------------------------------------------------
    
    const bool CMaterial::GetHasAlpha() const
    {
        return m_HasAlpha;
    }    

    // -----------------------------------------------------------------------------

    const bool CMaterial::GetHasBump() const
    {
        return m_HasBump;
    }
} // namespace Gfx