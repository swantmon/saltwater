
#include "engine/engine_precompiled.h"

#include "base/base_memory.h"

#include "engine/graphic/gfx_material.h"

namespace Gfx
{
    CMaterial::CMaterial()
        : m_Hash(0)
    {
        m_MaterialAttributes.m_Color[0] = 1.0f;
        m_MaterialAttributes.m_Color[1] = 1.0f;
        m_MaterialAttributes.m_Color[2] = 1.0f;
        m_MaterialAttributes.m_Color[3] = 1.0f;
        m_MaterialAttributes.m_Roughness = 0.0f;
        m_MaterialAttributes.m_Reflectance = 0.0f;
        m_MaterialAttributes.m_MetalMask = 0.0f;
        m_MaterialAttributes.m_Displacement = 0.0f;
        m_MaterialAttributes.m_TilingOffset[0] = 1.0f;
        m_MaterialAttributes.m_TilingOffset[1] = 1.0f;
        m_MaterialAttributes.m_TilingOffset[2] = 0.0f;
        m_MaterialAttributes.m_TilingOffset[3] = 0.0f;

        m_MaterialRefractionAttributes.m_IndexOfRefraction = 1.0f;

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

    const CMaterial::SRefractionAttributes& CMaterial::GetMaterialRefractionAttributes() const
    {
        return m_MaterialRefractionAttributes;
    }

    // -----------------------------------------------------------------------------

    const CMaterial::SMaterialKey& CMaterial::GetKey() const
    {
        return m_MaterialKey;
    }

    // -----------------------------------------------------------------------------

    bool CMaterial::HasAlpha() const
    {
        return m_MaterialAttributes.m_Color[3] < 1.0f || m_MaterialKey.m_HasAlphaTex;
    }

    // -----------------------------------------------------------------------------

    bool CMaterial::HasBump() const
    {
        return m_MaterialAttributes.m_Displacement > 0.0f && m_MaterialKey.m_HasBumpTex;
    }

    // -----------------------------------------------------------------------------

    bool CMaterial::HasRefraction() const
    {
        return m_MaterialRefractionAttributes.m_IndexOfRefraction != 1.0f;
    }

    // -----------------------------------------------------------------------------

    CMaterial::BHash CMaterial::GetHash() const
    {
        return m_Hash;
    }
} // namespace Gfx