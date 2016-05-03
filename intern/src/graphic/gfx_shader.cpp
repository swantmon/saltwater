
#include "graphic/gfx_precompiled.h"

#include "graphic/gfx_shader.h"

#include <assert.h>

namespace Gfx
{
    CShader::CShader()
        : m_ID            (s_InvalidID)
        , m_HasAlpha      (false)
        , m_InputLayoutPtr()
        , m_pFileName     ()
        , m_pShaderName   ()
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
        return m_pFileName;
    }
    
    // -----------------------------------------------------------------------------
    
    const Base::Char* CShader::GetShaderName() const
    {
        return m_pShaderName;
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

