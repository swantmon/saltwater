
#include "graphic/gfx_precompiled.h"

#include "graphic/gfx_shader.h"

#include <assert.h>

namespace Gfx
{
    CShader::CShader()
        : m_ID            (s_InvalidID)
        , m_HasAlpha      (false)
        , m_InputLayoutPtr()
        , m_FileName      ()
        , m_ShaderName    ()
        , m_ShaderDefines ()
        , m_Type          (UndefinedType)
    {
    }

    // -----------------------------------------------------------------------------

    CShader::~CShader()
    {
    }

    // -----------------------------------------------------------------------------

    unsigned int CShader::GetID() const
    {
        return m_ID;
    }
    
    // -----------------------------------------------------------------------------
    
    CShader::EType CShader::GetType() const
    {
        return m_Type;
    }
    
    // -----------------------------------------------------------------------------
    
    const Base::Char* CShader::GetFileName() const
    {
        return m_FileName.c_str();
    }
    
    // -----------------------------------------------------------------------------
    
    const Base::Char* CShader::GetShaderName() const
    {
        return m_ShaderName.c_str();
    }

    // -----------------------------------------------------------------------------

    const Base::Char* CShader::GetShaderDefines() const
    {
        return m_ShaderDefines.c_str();
    }

    // -----------------------------------------------------------------------------

    bool CShader::HasAlpha() const
    {
        return m_HasAlpha;
    }

    // -----------------------------------------------------------------------------

    CInputLayoutPtr CShader::GetInputLayout()
    {
        return m_InputLayoutPtr;
    }
} // namespace Gfx

