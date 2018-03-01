
#include "graphic/gfx_precompiled.h"

#include "base/base_memory.h"

#include "graphic/gfx_material.h"

namespace Gfx
{
    CMaterial::CMaterial()
        : m_HasAlpha(false)
        , m_HasBump(false)
        , m_Hash(0)
    {
        m_MaterialAttributes.m_Color[0] = 1.0f;
        m_MaterialAttributes.m_Color[1] = 1.0f;
        m_MaterialAttributes.m_Color[2] = 1.0f;
        m_MaterialAttributes.m_Roughness = 0.0f;
        m_MaterialAttributes.m_Reflectance = 0.0f;
        m_MaterialAttributes.m_MetalMask = 0.0f;
        m_MaterialAttributes.m_Displacement = 0.0f;
        m_MaterialAttributes.m_TilingOffset[0] = 1.0f;
        m_MaterialAttributes.m_TilingOffset[1] = 1.0f;
        m_MaterialAttributes.m_TilingOffset[2] = 0.0f;
        m_MaterialAttributes.m_TilingOffset[3] = 0.0f;

        m_MaterialKey.m_Key = 0;

        m_ShaderPtrs[CShader::Vertex] = 0;
        m_ShaderPtrs[CShader::Hull] = 0;
        m_ShaderPtrs[CShader::Domain] = 0;
        m_ShaderPtrs[CShader::Geometry] = 0;
        m_ShaderPtrs[CShader::Pixel] = 0;

        m_ForwardShaderPSPtr = 0;

        m_SamplerSetPtr = 0;

        m_TextureSetPtr = 0;
    }

    // -----------------------------------------------------------------------------

    CMaterial::~CMaterial()
    {
        m_ShaderPtrs[CShader::Vertex] = 0;
        m_ShaderPtrs[CShader::Hull] = 0;
        m_ShaderPtrs[CShader::Domain] = 0;
        m_ShaderPtrs[CShader::Geometry] = 0;
        m_ShaderPtrs[CShader::Pixel] = 0;

        m_ForwardShaderPSPtr = 0;

        m_SamplerSetPtr = 0;

        m_TextureSetPtr = 0;
    }

    // -----------------------------------------------------------------------------

    CShaderPtr CMaterial::GetShaderGS() const
    {
        return m_ShaderPtrs[CShader::Geometry];
    }

    // -----------------------------------------------------------------------------

    CShaderPtr CMaterial::GetShaderDS() const
    {
        return m_ShaderPtrs[CShader::Domain];
    }

    // -----------------------------------------------------------------------------

    CShaderPtr CMaterial::GetShaderHS() const
    {
        return m_ShaderPtrs[CShader::Hull];
    }

    // -----------------------------------------------------------------------------

    CShaderPtr CMaterial::GetShaderPS() const
    {
        return m_ShaderPtrs[CShader::Pixel];
    }

    // -----------------------------------------------------------------------------

    CShaderPtr CMaterial::GetForwardShaderPS() const
    {
        return m_ForwardShaderPSPtr;
    }

    // -----------------------------------------------------------------------------

    CSamplerSetPtr CMaterial::GetSamplerSetPS() const
    {
        return m_SamplerSetPtr;
    }

    // -----------------------------------------------------------------------------

    CTextureSetPtr CMaterial::GetTextureSetPS() const
    {
        return m_TextureSetPtr;
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

    bool CMaterial::GetHasAlpha() const
    {
        return m_HasAlpha;
    }

    // -----------------------------------------------------------------------------

    bool CMaterial::GetHasBump() const
    {
        return m_HasBump && m_MaterialAttributes.m_Displacement > 0.0f;
    }

    // -----------------------------------------------------------------------------

    unsigned int CMaterial::GetHash() const
    {
        return m_Hash;
    }
} // namespace Gfx