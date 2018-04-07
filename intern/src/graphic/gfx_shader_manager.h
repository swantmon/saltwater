
#pragma once

#include "engine/engine_config.h"

#include "base/base_typedef.h"

#include "graphic/gfx_input_layout.h"
#include "graphic/gfx_shader.h"

namespace Gfx
{
    struct SInputElementDescriptor
    {
        const Base::Char*                  m_pSemanticName;
        unsigned int                       m_SemanticIndex;
        CInputLayout::EFormat              m_Format;
        unsigned int                       m_InputSlot;
        unsigned int                       m_AlignedByteOffset;
        unsigned int                       m_Stride;
        CInputLayout::EInputClassification m_InputSlotClass;
        unsigned int                       m_InstanceDataStepRate;
    };
} // namespace Gfx

namespace Gfx
{
namespace ShaderManager
{
    void OnStart();
    void OnExit();

    ENGINE_API CShaderPtr CompileVS(const Base::Char* _pFileName, const Base::Char* _pShaderName, const Base::Char* _pShaderDefines = nullptr, const Base::Char* _pShaderDescription = nullptr, unsigned int _Categories = 0, bool _HasAlpha = false, bool _Debug = false, bool _IsCode = false);
    ENGINE_API CShaderPtr CompileGS(const Base::Char* _pFileName, const Base::Char* _pShaderName, const Base::Char* _pShaderDefines = nullptr, const Base::Char* _pShaderDescription = nullptr, unsigned int _Categories = 0, bool _HasAlpha = false, bool _Debug = false, bool _IsCode = false);
    ENGINE_API CShaderPtr CompileDS(const Base::Char* _pFileName, const Base::Char* _pShaderName, const Base::Char* _pShaderDefines = nullptr, const Base::Char* _pShaderDescription = nullptr, unsigned int _Categories = 0, bool _HasAlpha = false, bool _Debug = false, bool _IsCode = false);
    ENGINE_API CShaderPtr CompileHS(const Base::Char* _pFileName, const Base::Char* _pShaderName, const Base::Char* _pShaderDefines = nullptr, const Base::Char* _pShaderDescription = nullptr, unsigned int _Categories = 0, bool _HasAlpha = false, bool _Debug = false, bool _IsCode = false);
    ENGINE_API CShaderPtr CompilePS(const Base::Char* _pFileName, const Base::Char* _pShaderName, const Base::Char* _pShaderDefines = nullptr, const Base::Char* _pShaderDescription = nullptr, unsigned int _Categories = 0, bool _HasAlpha = false, bool _Debug = false, bool _IsCode = false);
    ENGINE_API CShaderPtr CompileCS(const Base::Char* _pFileName, const Base::Char* _pShaderName, const Base::Char* _pShaderDefines = nullptr, const Base::Char* _pShaderDescription = nullptr, unsigned int _Categories = 0, bool _HasAlpha = false, bool _Debug = false, bool _IsCode = false);

    ENGINE_API void ReloadShader(CShaderPtr _ShaderPtr);
    ENGINE_API void ReloadAllShaders();

    ENGINE_API CInputLayoutPtr CreateInputLayout(const SInputElementDescriptor* _pDescriptors, unsigned int _NumberOfDescriptors, CShaderPtr _ShaderVSPtr);

    ENGINE_API void SetShaderLabel(CShaderPtr _ShaderPtr, const char* _pLabel);
} // namespace ShaderManager
} // namespace Gfx