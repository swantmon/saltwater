
#include "engine/engine_precompiled.h"

#include "base/base_crc.h"
#include "base/base_exception.h"
#include "base/base_managed_pool.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "core/core_asset_manager.h"
#include "core/core_console.h"
#include "core/core_program_parameters.h"

#include "graphic/gfx_main.h"
#include "graphic/gfx_native_shader.h"
#include "graphic/gfx_native_types.h"
#include "graphic/gfx_shader_manager.h"

#include <assert.h>
#include <exception>
#include <fstream>
#include <string>
#include <unordered_map>

using namespace Gfx;
using namespace Gfx::ShaderManager;

namespace
{
    static const char* g_PathToDataShader = "/graphic/shaders/";
} // namespace

namespace
{
    class CGfxShaderManager : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGfxShaderManager)

    public:

        CGfxShaderManager();
        ~CGfxShaderManager();

    public:

        void OnStart();
        void OnExit();

    public:

        CShaderPtr CompileVS(const Base::Char* _pFileName, const Base::Char* _pShaderName, const Base::Char* _pShaderDefines, const Base::Char* _pShaderDescription, unsigned int _Categories, bool _HasAlpha, bool _Debug, bool _IsCode);
        CShaderPtr CompileGS(const Base::Char* _pFileName, const Base::Char* _pShaderName, const Base::Char* _pShaderDefines, const Base::Char* _pShaderDescription, unsigned int _Categories, bool _HasAlpha, bool _Debug, bool _IsCode);
        CShaderPtr CompileDS(const Base::Char* _pFileName, const Base::Char* _pShaderName, const Base::Char* _pShaderDefines, const Base::Char* _pShaderDescription, unsigned int _Categories, bool _HasAlpha, bool _Debug, bool _IsCode);
        CShaderPtr CompileHS(const Base::Char* _pFileName, const Base::Char* _pShaderName, const Base::Char* _pShaderDefines, const Base::Char* _pShaderDescription, unsigned int _Categories, bool _HasAlpha, bool _Debug, bool _IsCode);
        CShaderPtr CompilePS(const Base::Char* _pFileName, const Base::Char* _pShaderName, const Base::Char* _pShaderDefines, const Base::Char* _pShaderDescription, unsigned int _Categories, bool _HasAlpha, bool _Debug, bool _IsCode);
        CShaderPtr CompileCS(const Base::Char* _pFileName, const Base::Char* _pShaderName, const Base::Char* _pShaderDefines, const Base::Char* _pShaderDescription, unsigned int _Categories, bool _HasAlpha, bool _Debug, bool _IsCode);

    public:

        void ReloadShader(CShaderPtr _ShaderPtr);
        void ReloadAllShaders();

    public:

        CShaderPtr GetShader(const Base::Char* _pFileName, const Base::Char* _pShaderName, CShader::EType _Type, bool _Debug);

    public:

        CInputLayoutPtr CreateInputLayout(const SInputElementDescriptor* _pDescriptors, unsigned int _NumberOfDescriptors, CShaderPtr _ShaderVSPtr);

    public:

        void SetShaderLabel(CShaderPtr _ShaderPtr, const char* _pLabel);

    private:

        // -----------------------------------------------------------------------------
        // Represents an unique shader.
        // -----------------------------------------------------------------------------
        class CInternShader : public CNativeShader
        {
        public:

            CInternShader();
           ~CInternShader();

        private:

            bool m_Debug;
            unsigned int m_Hash;

        private:

            friend class CGfxShaderManager;
        };

    private:

        // -----------------------------------------------------------------------------
        // Represents an unique input layout.
        // -----------------------------------------------------------------------------
        class CInternInputLayout : public CInputLayout
        {
            public:

                CInternInputLayout();
               ~CInternInputLayout();

            private:

                friend class CGfxShaderManager;
        };

        typedef Base::CManagedPool<CInternShader, 128>      CShaders;
        typedef CShaders::CIterator                         CShaderIterator;
        typedef Base::CManagedPool<CInternInputLayout, 128> CInputLayouts;
        typedef CInputLayouts::CIterator                    CInputLayoutIterator;

        typedef std::unordered_map<unsigned int, CShaderPtr> CShaderByIDs;

    private:

        CInputLayouts m_InputLayouts;
        CShaders      m_Shaders[CShader::NumberOfTypes];

        CShaderByIDs    m_ShaderByID;

    private:

        CShaderPtr InternCompileShader(CShader::EType _Type, const Base::Char* _pFileName, const Base::Char* _pShaderName, const Base::Char* _pShaderDefines, const Base::Char* _pShaderDescription, unsigned int _Categories, bool _HasAlpha, bool _Debug, bool _IsCode);

        void InternReloadShader(CInternShader* _pInternShader);

        void PreprocessorShader(std::string& _rShaderContent, bool _AddVersion);

        int ConvertShaderType(CShader::EType _Type);

    private:

        friend class CIterator;
    };
} // namespace

namespace
{
    CGfxShaderManager::CGfxShaderManager()
        : m_InputLayouts()
        , m_Shaders     ()
        , m_ShaderByID  ()
    {
        m_ShaderByID.reserve(128);
    }

    // -----------------------------------------------------------------------------

    CGfxShaderManager::~CGfxShaderManager()
    {
    }

    // -----------------------------------------------------------------------------

    void CGfxShaderManager::OnStart()
    {
    }

    // -----------------------------------------------------------------------------

    void CGfxShaderManager::OnExit()
    {
        unsigned int ShaderType;

        m_ShaderByID.clear();

        m_InputLayouts.Clear();

        for (ShaderType = 0; ShaderType < CShader::NumberOfTypes; ++ ShaderType)
        {
            m_Shaders[ShaderType].Clear();
        }
    }

    // -----------------------------------------------------------------------------

    CShaderPtr CGfxShaderManager::CompileVS(const Base::Char* _pFileName, const Base::Char* _pShaderName, const Base::Char* _pShaderDefines, const Base::Char* _pShaderDescription, unsigned int _Categories, bool _HasAlpha, bool _Debug, bool _IsCode)
    {
        return InternCompileShader(Gfx::CShader::Vertex, _pFileName, _pShaderName, _pShaderDefines, _pShaderDescription, _Categories, _HasAlpha, _Debug, _IsCode);
    }

    // -----------------------------------------------------------------------------

    CShaderPtr CGfxShaderManager::CompileGS(const Base::Char* _pFileName, const Base::Char* _pShaderName, const Base::Char* _pShaderDefines, const Base::Char* _pShaderDescription, unsigned int _Categories, bool _HasAlpha, bool _Debug, bool _IsCode)
    {
        return InternCompileShader(Gfx::CShader::Geometry, _pFileName, _pShaderName, _pShaderDefines, _pShaderDescription, _Categories, _HasAlpha, _Debug, _IsCode);
    }

    // -----------------------------------------------------------------------------

    CShaderPtr CGfxShaderManager::CompileDS(const Base::Char* _pFileName, const Base::Char* _pShaderName, const Base::Char* _pShaderDefines, const Base::Char* _pShaderDescription, unsigned int _Categories, bool _HasAlpha, bool _Debug, bool _IsCode)
    {
        return InternCompileShader(Gfx::CShader::Domain, _pFileName, _pShaderName, _pShaderDefines, _pShaderDescription, _Categories, _HasAlpha, _Debug, _IsCode);
    }

    // -----------------------------------------------------------------------------

    CShaderPtr CGfxShaderManager::CompileHS(const Base::Char* _pFileName, const Base::Char* _pShaderName, const Base::Char* _pShaderDefines, const Base::Char* _pShaderDescription, unsigned int _Categories, bool _HasAlpha, bool _Debug, bool _IsCode)
    {
        return InternCompileShader(Gfx::CShader::Hull, _pFileName, _pShaderName, _pShaderDefines, _pShaderDescription, _Categories, _HasAlpha, _Debug, _IsCode);
    }

    // -----------------------------------------------------------------------------

    CShaderPtr CGfxShaderManager::CompilePS(const Base::Char* _pFileName, const Base::Char* _pShaderName, const Base::Char* _pShaderDefines, const Base::Char* _pShaderDescription, unsigned int _Categories, bool _HasAlpha, bool _Debug, bool _IsCode)
    {
        return InternCompileShader(Gfx::CShader::Pixel, _pFileName, _pShaderName, _pShaderDefines, _pShaderDescription, _Categories, _HasAlpha, _Debug, _IsCode);
    }

    // -----------------------------------------------------------------------------

    CShaderPtr CGfxShaderManager::CompileCS(const Base::Char* _pFileName, const Base::Char* _pShaderName, const Base::Char* _pShaderDefines, const Base::Char* _pShaderDescription, unsigned int _Categories, bool _HasAlpha, bool _Debug, bool _IsCode)
    {
       return InternCompileShader(Gfx::CShader::Compute, _pFileName, _pShaderName, _pShaderDefines, _pShaderDescription, _Categories, _HasAlpha, _Debug, _IsCode);
    }

    // -----------------------------------------------------------------------------

    void CGfxShaderManager::ReloadShader(CShaderPtr _ShaderPtr)
    {
        assert(_ShaderPtr != nullptr && _ShaderPtr.IsValid());

        InternReloadShader(static_cast<CInternShader*>(_ShaderPtr.GetPtr()));
    }

    // -----------------------------------------------------------------------------

    void CGfxShaderManager::ReloadAllShaders()
    {
        for (unsigned int IndexOfType = 0; IndexOfType < CShader::NumberOfTypes; ++IndexOfType)
        {
            CShaderIterator CurrentShader = m_Shaders[IndexOfType].Begin();
            CShaderIterator EndOfShaders  = m_Shaders[IndexOfType].End();

            for (; CurrentShader != EndOfShaders; ++CurrentShader)
            {
                InternReloadShader(&(*CurrentShader));
            }
        }
    }

    // -----------------------------------------------------------------------------

    CShaderPtr CGfxShaderManager::GetShader(const Base::Char* _pFileName, const Base::Char* _pShaderName, CShader::EType _Type, bool _Debug)
    {
        BASE_UNUSED(_pFileName);
        BASE_UNUSED(_pShaderName);
        BASE_UNUSED(_Type);
        BASE_UNUSED(_Debug);

        return CShaderPtr();
    }

    // -----------------------------------------------------------------------------

    CInputLayoutPtr CGfxShaderManager::CreateInputLayout(const SInputElementDescriptor* _pDescriptors, unsigned int _NumberOfDescriptors, CShaderPtr _VertexShaderPtr)
    {
        unsigned int IndexOfDescriptor;

        assert((_pDescriptors != nullptr) && (_NumberOfDescriptors <= CInputLayout::s_MaxNumberOfElements) && _VertexShaderPtr != 0);

        // -----------------------------------------------------------------------------
        // Create a temporary input layout to check if it already exists.
        // -----------------------------------------------------------------------------
        CInputLayouts::CPtr InputLayoutPtr = m_InputLayouts.Allocate();

        CInternInputLayout& rInputLayout = *InputLayoutPtr;

        // -----------------------------------------------------------------------------
        // Note that in case of an exception the resource is automatically freed because
        // the smart pointer is not returned but deleted.
        // -----------------------------------------------------------------------------
        try
        {
            rInputLayout.m_NumberOfElements = _NumberOfDescriptors;

            for (IndexOfDescriptor = 0; IndexOfDescriptor < _NumberOfDescriptors; ++ IndexOfDescriptor)
            {
                const SInputElementDescriptor& rInputElementDescriptor = _pDescriptors          [IndexOfDescriptor];
                CInputLayout::CElement&        rInputElement           = rInputLayout.m_Elements[IndexOfDescriptor];

                rInputLayout.SetSemanticName        (rInputElement, rInputElementDescriptor.m_pSemanticName);
                rInputLayout.SetSemanticIndex       (rInputElement, rInputElementDescriptor.m_SemanticIndex);
                rInputLayout.SetFormat              (rInputElement, rInputElementDescriptor.m_Format);
                rInputLayout.SetInputSlot           (rInputElement, rInputElementDescriptor.m_InputSlot);
                rInputLayout.SetAlignedByteOffset   (rInputElement, rInputElementDescriptor.m_AlignedByteOffset);
                rInputLayout.SetStride              (rInputElement, rInputElementDescriptor.m_Stride);
                rInputLayout.SetInputClassification (rInputElement, rInputElementDescriptor.m_InputSlotClass);
                rInputLayout.SetInstanceDataStepRate(rInputElement, rInputElementDescriptor.m_InstanceDataStepRate);
            }

            // -----------------------------------------------------------------------------
            // Set input layout to vertex shader
            // -----------------------------------------------------------------------------
            CInternShader& rCurrentShader = *static_cast<CInternShader*>(_VertexShaderPtr.GetPtr());

            rCurrentShader.m_InputLayoutPtr = CInputLayoutPtr(InputLayoutPtr);
        }
        catch (...)
        {
            BASE_THROWM("Error while creating input layout");
        }

        return CInputLayoutPtr(InputLayoutPtr);
    }

    // -----------------------------------------------------------------------------

    void CGfxShaderManager::SetShaderLabel(CShaderPtr _ShaderPtr, const char* _pLabel)
    {
        assert(_pLabel != nullptr);

        CInternShader* pInternShader = static_cast<CInternShader*>(_ShaderPtr.GetPtr());

        glObjectLabel(GL_SHADER, pInternShader->m_NativeShader, -1, _pLabel);
    }

    // -----------------------------------------------------------------------------

    CShaderPtr CGfxShaderManager::InternCompileShader(CShader::EType _Type, const Base::Char* _pFileName, const Base::Char* _pShaderName, const Base::Char* _pShaderDefines, const Base::Char* _pShaderDescription, unsigned int _Categories, bool _HasAlpha, bool _Debug, bool _IsCode)
    {
        BASE_UNUSED(_pShaderDescription);
        BASE_UNUSED(_Categories);

        assert(_pFileName   != 0);
        assert(_pShaderName != 0);

        // -----------------------------------------------------------------------------
        // Create hash and try to take an existing shader
        // -----------------------------------------------------------------------------
        unsigned int Hash = Base::CRC32(_pFileName, static_cast<unsigned int>(strlen(_pFileName)));
        Hash              = Base::CRC32(Hash, _pShaderName, static_cast<unsigned int>(strlen(_pShaderName)));
        Hash              = Base::CRC32(Hash, &_Type, sizeof(CShader::EType));

        if (_pShaderDefines != 0)
        {
            Hash = Base::CRC32(Hash, _pShaderDefines, static_cast<unsigned int>(strlen(_pShaderDefines)));
        }

        if (m_ShaderByID.find(Hash) != m_ShaderByID.end())
        {
            return m_ShaderByID[Hash];
        }

        // -----------------------------------------------------------------------------
        // Load file data from given filename
        // -----------------------------------------------------------------------------
        std::string ShaderContent;
        std::string ShaderLabel;

        if (_IsCode)
        {
            ShaderContent = std::string(_pFileName);

            ShaderLabel = "Internal Shader : " + std::string(_pShaderName);
        }
        else
        {
            std::string PathToShader = Core::AssetManager::GetPathToData() + g_PathToDataShader + _pFileName;

            std::ifstream ShaderFile(PathToShader.c_str());

            if (!ShaderFile.is_open())
            {
                BASE_THROWV("Shader '%s' can't be opened!", PathToShader.c_str());
            }

            ShaderContent = std::string((std::istreambuf_iterator<char>(ShaderFile)), std::istreambuf_iterator<char>());

            ShaderLabel = std::string(_pFileName) + " : " + std::string(_pShaderName);
        }

        if (_pShaderDefines != 0)
        {
            ShaderContent = std::string(_pShaderDefines) + "\n" + ShaderContent;
        }

        if (!_IsCode)
        {
            ShaderContent = "#define " + std::string(_pShaderName) + " main\n" + ShaderContent;

            PreprocessorShader(ShaderContent, true);
        }

        const char* pRAW = ShaderContent.c_str();

        // -----------------------------------------------------------------------------
        // Create and compile shader
        //
        // Warning: When linking shaders with separable programs, your shaders must
        // redeclare the gl_PerVertex interface block if you attempt to use any of
        // the variables defined within it.
        // -----------------------------------------------------------------------------
        GLuint NativeProgramHandle = 0;
        GLuint NativeShaderHandle  = 0;
        GLint  Error;

        NativeShaderHandle = glCreateShader(ConvertShaderType(_Type));

        if (NativeShaderHandle != 0)
        {
            glObjectLabel(GL_SHADER, NativeShaderHandle, -1, ShaderLabel.c_str());

            glShaderSource(NativeShaderHandle, 1, &pRAW, NULL);

            glCompileShader(NativeShaderHandle);

            glGetShaderiv(NativeShaderHandle, GL_COMPILE_STATUS, &Error);

            if (!Error)
            {
                GLint InfoLength = 0;
                glGetShaderiv(NativeShaderHandle, GL_INFO_LOG_LENGTH, &InfoLength);

                char* pErrorInfo = new char[InfoLength];
                glGetShaderInfoLog(NativeShaderHandle, InfoLength, &InfoLength, pErrorInfo);

                ENGINE_CONSOLE_ERRORV("Error creating shader '%s' with error log:\n%s\n", ShaderLabel.c_str(), pErrorInfo);

// #define GFX_SHADER_SHOW_SOURCE_ON_ERROR
#ifdef GFX_SHADER_SHOW_SOURCE_ON_ERROR
                ENGINE_CONSOLE_INFO("Full source code of shader:");
                std::stringstream Line;
                int LineNumber = 0;

                for (int i = 0; i < strlen(pRAW); ++i)
                {
                    if (pRAW[i] == '\n')
                    {
                        ENGINE_CONSOLE_INFOV("%i: %s", LineNumber, Line.str().c_str());

                        Line.str("");

                        LineNumber ++;

                        continue;
                    }

                    Line << pRAW[i];
                }
#endif

                delete[] pErrorInfo;
            }

            NativeProgramHandle = glCreateProgram();

            if (NativeProgramHandle != 0)
            {
                GLint CompileStatus;

                glGetShaderiv(NativeShaderHandle, GL_COMPILE_STATUS, &CompileStatus);

                glProgramParameteri(NativeProgramHandle, GL_PROGRAM_SEPARABLE, GL_TRUE);

                if (CompileStatus != 0)
                {
                    glAttachShader(NativeProgramHandle, NativeShaderHandle);

                    glLinkProgram(NativeProgramHandle);

                    glDetachShader(NativeProgramHandle, NativeShaderHandle);
                }

                glGetProgramiv(NativeProgramHandle, GL_LINK_STATUS, &Error);

                if (!Error)
                {
                    GLint InfoLength = 0;
                    glGetProgramiv(NativeProgramHandle, GL_INFO_LOG_LENGTH, &InfoLength);

                    char* pErrorInfo = new char[InfoLength];
                    glGetProgramInfoLog(NativeProgramHandle, InfoLength, &InfoLength, pErrorInfo);

                    ENGINE_CONSOLE_ERRORV("Error creating a shader program for '%s' and linking shader: \n %s", ShaderLabel.c_str(), pErrorInfo);

                    delete[] pErrorInfo;

                    glDeleteProgram(NativeProgramHandle);
                }
            }
        }
    
        glDeleteShader(NativeShaderHandle);

        // -----------------------------------------------------------------------------
        // Create shader
        // -----------------------------------------------------------------------------
        CShaderPtr ShaderPtr = static_cast<CShaderPtr>(m_Shaders[_Type].Allocate());

        CInternShader& rShader = *static_cast<CInternShader*>(ShaderPtr.GetPtr());

        // -----------------------------------------------------------------------------
        // Setup the engine shader
        // -----------------------------------------------------------------------------
        rShader.m_ID             = m_Shaders[_Type].GetNumberOfItems();
        rShader.m_HasAlpha       = _HasAlpha;
        rShader.m_FileName       = _pFileName;
        rShader.m_ShaderName     = _pShaderName;
        rShader.m_Type           = _Type;
        rShader.m_Debug          = _Debug;
        rShader.m_Hash           = Hash;
        rShader.m_NativeShader   = NativeProgramHandle;

        if (_pShaderDefines != 0) rShader.m_ShaderDefines = _pShaderDefines;

        // -----------------------------------------------------------------------------
        // Set current shader into hash map
        // -----------------------------------------------------------------------------
        if (Hash != 0)
        {
            m_ShaderByID[Hash] = ShaderPtr;
        }

        return ShaderPtr;
    }

    // -----------------------------------------------------------------------------

    void CGfxShaderManager::InternReloadShader(CInternShader* _pInternShader)
    {
        assert(_pInternShader != nullptr);

        CInternShader& rShader = *_pInternShader;

        // -----------------------------------------------------------------------------
        // Remove old shader
        // -----------------------------------------------------------------------------
        glDeleteProgram(rShader.m_NativeShader);

        // -----------------------------------------------------------------------------
        // Build path to shader in file system
        // -----------------------------------------------------------------------------
        std::string PathToShader = Core::AssetManager::GetPathToData() + g_PathToDataShader + _pInternShader->GetFileName();

        // -----------------------------------------------------------------------------
        // Load file data from given filename
        // -----------------------------------------------------------------------------
        Base::Size ShaderLength;

        std::ifstream ShaderFile(PathToShader.c_str());

        assert(ShaderFile.is_open());

        std::string ShaderFileContent((std::istreambuf_iterator<char>(ShaderFile)), std::istreambuf_iterator<char>());

        if (rShader.m_ShaderDefines.length() > 0)
        {
            ShaderFileContent = rShader.m_ShaderDefines + "\n" + ShaderFileContent;
        }

        ShaderFileContent = "#define " + rShader.m_ShaderName + " main\n" + ShaderFileContent;

        PreprocessorShader(ShaderFileContent, true);
        
        ShaderLength = ShaderFileContent.size();

        const char* pRAW = ShaderFileContent.c_str();

        // -----------------------------------------------------------------------------
        // Create and compile shader
        //
        // Warning: When linking shaders with separable programs, your shaders must
        // redeclare the gl_PerVertex interface block if you attempt to use any of
        // the variables defined within it.
        // -----------------------------------------------------------------------------
        GLuint NativeProgramHandle = 0;
        GLuint NativeShaderHandle = 0;
        GLint  Error;

        NativeShaderHandle = glCreateShader(ConvertShaderType(rShader.m_Type));

        if (NativeShaderHandle != 0)
        {
            glObjectLabel(GL_SHADER, NativeShaderHandle, -1, (rShader.m_FileName + " : " + rShader.m_ShaderName).c_str());

            glShaderSource(NativeShaderHandle, 1, &pRAW, NULL);

            glCompileShader(NativeShaderHandle);

            glGetShaderiv(NativeShaderHandle, GL_COMPILE_STATUS, &Error);

            if (!Error)
            {
                GLint InfoLength = 0;
                glGetShaderiv(NativeShaderHandle, GL_INFO_LOG_LENGTH, &InfoLength);

                char* pErrorInfo = new char[InfoLength];
                glGetShaderInfoLog(NativeShaderHandle, InfoLength, &InfoLength, pErrorInfo);

                ENGINE_CONSOLE_ERRORV("Error creating shader '%s' with info: \n %s", PathToShader.c_str(), pErrorInfo);

                delete[] pErrorInfo;
            }

            NativeProgramHandle = glCreateProgram();

            if (NativeProgramHandle != 0)
            {
                GLint CompileStatus;

                glGetShaderiv(NativeShaderHandle, GL_COMPILE_STATUS, &CompileStatus);

                glProgramParameteri(NativeProgramHandle, GL_PROGRAM_SEPARABLE, GL_TRUE);

                if (CompileStatus != 0)
                {
                    glAttachShader(NativeProgramHandle, NativeShaderHandle);

                    glLinkProgram(NativeProgramHandle);

                    glDetachShader(NativeProgramHandle, NativeShaderHandle);
                }

                glGetProgramiv(NativeProgramHandle, GL_LINK_STATUS, &Error);

                if (!Error)
                {
                    GLint InfoLength = 0;
                    glGetProgramiv(NativeProgramHandle, GL_INFO_LOG_LENGTH, &InfoLength);

                    char* pErrorInfo = new char[InfoLength];
                    glGetProgramInfoLog(NativeProgramHandle, InfoLength, &InfoLength, pErrorInfo);

                    ENGINE_CONSOLE_ERRORV("Error creating a shader program for '%s' and linking shader: \n %s", PathToShader.c_str(), pErrorInfo);

                    delete[] pErrorInfo;

                    glDeleteProgram(NativeProgramHandle);
                }
            }
        }

        glDeleteShader(NativeShaderHandle);

        // -----------------------------------------------------------------------------
        // Setup the engine shader
        // -----------------------------------------------------------------------------
        rShader.m_NativeShader = NativeProgramHandle;
    }

    // -----------------------------------------------------------------------------

    void CGfxShaderManager::PreprocessorShader(std::string& _rShaderContent, bool _AddVersion)
    {
        Base::Size FoundPosition     = 0;
        Base::Size UndefinedPosition = std::string::npos;

        while (FoundPosition != UndefinedPosition)
        {
            FoundPosition = _rShaderContent.find("#include", FoundPosition);

            if (FoundPosition != UndefinedPosition)
            {
                Base::Size BeginOfInclude = _rShaderContent.find('\"', FoundPosition) + 1;
                Base::Size EndOfInclude   = _rShaderContent.find('\"', BeginOfInclude);

                std::string IncludeFile  = Core::AssetManager::GetPathToData() + g_PathToDataShader + _rShaderContent.substr(BeginOfInclude, EndOfInclude - BeginOfInclude);

                // -----------------------------------------------------------------------------
                // Load included file and replace include directive with new file
                // content. Furthermore check new file content on include directives.
                // -----------------------------------------------------------------------------
                std::ifstream ShaderFile(IncludeFile.c_str());

                std::string ShaderFileContent((std::istreambuf_iterator<char>(ShaderFile)), std::istreambuf_iterator<char>());

                PreprocessorShader(ShaderFileContent, false);

                _rShaderContent.replace(FoundPosition, EndOfInclude - FoundPosition + 1, ShaderFileContent);

                FoundPosition += 1;
            }
        }

        if (_AddVersion)
        {
            if (Main::GetGraphicsAPI().m_GraphicsAPI == CGraphicsInfo::OpenGLES)
            {
                _rShaderContent = "precision lowp sampler2D; \n" + _rShaderContent;
                _rShaderContent = "precision lowp sampler3D; \n" + _rShaderContent;
                _rShaderContent = "precision lowp samplerCube; \n" + _rShaderContent;
                _rShaderContent = "precision lowp image2D; \n" + _rShaderContent;
                _rShaderContent = "precision lowp image3D; \n" + _rShaderContent;
                _rShaderContent = "precision lowp uimage2D; \n" + _rShaderContent;
                _rShaderContent = "precision lowp sampler2DShadow; \n" + _rShaderContent;
                _rShaderContent = "precision highp float; \n" + _rShaderContent;
                _rShaderContent = "#version 320 es \n" + _rShaderContent;
            }
            else
            {
                _rShaderContent = "#version 450 \n" + _rShaderContent;
            }
        }
    }

    // -----------------------------------------------------------------------------

    int CGfxShaderManager::ConvertShaderType(CShader::EType _Type)
    {
        static int s_NativeShaderType[] =
        {
            GL_VERTEX_SHADER,
            GL_TESS_CONTROL_SHADER,
            GL_TESS_EVALUATION_SHADER,
            GL_GEOMETRY_SHADER,
            GL_FRAGMENT_SHADER,
            GL_COMPUTE_SHADER,
        };

        return s_NativeShaderType[_Type];
    }

} // namespace

namespace
{
    CGfxShaderManager::CInternShader::CInternShader()
        : CNativeShader()
        , m_Debug      (false)
        , m_Hash       (0)
    {
    }

    // -----------------------------------------------------------------------------

    CGfxShaderManager::CInternShader::~CInternShader()
    {
        glDeleteProgram(m_NativeShader);
    }
} // namespace

namespace
{
    CGfxShaderManager::CInternInputLayout::CInternInputLayout()
        : CInputLayout()
    {
    }

    // -----------------------------------------------------------------------------

    CGfxShaderManager::CInternInputLayout::~CInternInputLayout()
    {
    }
} // namespace

namespace Gfx
{
namespace ShaderManager
{
    void OnStart()
    {
        CGfxShaderManager::GetInstance().OnStart();
    }

    // -----------------------------------------------------------------------------

    void OnExit()
    {
        CGfxShaderManager::GetInstance().OnExit();
    }

    // -----------------------------------------------------------------------------

    CShaderPtr CompileVS(const Base::Char* _pFileName, const Base::Char* _pShaderName, const Base::Char* _pShaderDefines, const Base::Char* _pShaderDescription, unsigned int _Categories, bool _HasAlpha, bool _Debug, bool _IsCode)
    {
        return CGfxShaderManager::GetInstance().CompileVS(_pFileName, _pShaderName, _pShaderDefines, _pShaderDescription, _Categories, _HasAlpha, _Debug, _IsCode);
    }

    // -----------------------------------------------------------------------------

    CShaderPtr CompileGS(const Base::Char* _pFileName, const Base::Char* _pShaderName, const Base::Char* _pShaderDefines, const Base::Char* _pShaderDescription, unsigned int _Categories, bool _HasAlpha, bool _Debug, bool _IsCode)
    {
        return CGfxShaderManager::GetInstance().CompileGS(_pFileName, _pShaderName, _pShaderDefines, _pShaderDescription, _Categories, _HasAlpha, _Debug, _IsCode);
    }

    // -----------------------------------------------------------------------------

    CShaderPtr CompileDS(const Base::Char* _pFileName, const Base::Char* _pShaderName, const Base::Char* _pShaderDefines, const Base::Char* _pShaderDescription, unsigned int _Categories, bool _HasAlpha, bool _Debug, bool _IsCode)
    {
        return CGfxShaderManager::GetInstance().CompileDS(_pFileName, _pShaderName, _pShaderDefines, _pShaderDescription, _Categories, _HasAlpha, _Debug, _IsCode);
    }

    // -----------------------------------------------------------------------------

    CShaderPtr CompileHS(const Base::Char* _pFileName, const Base::Char* _pShaderName, const Base::Char* _pShaderDefines, const Base::Char* _pShaderDescription, unsigned int _Categories, bool _HasAlpha, bool _Debug, bool _IsCode)
    {
        return CGfxShaderManager::GetInstance().CompileHS(_pFileName, _pShaderName, _pShaderDefines, _pShaderDescription, _Categories, _HasAlpha, _Debug, _IsCode);
    }

    // -----------------------------------------------------------------------------

    CShaderPtr CompilePS(const Base::Char* _pFileName, const Base::Char* _pShaderName, const Base::Char* _pShaderDefines, const Base::Char* _pShaderDescription, unsigned int _Categories, bool _HasAlpha, bool _Debug, bool _IsCode)
    {
        return CGfxShaderManager::GetInstance().CompilePS(_pFileName, _pShaderName, _pShaderDefines, _pShaderDescription, _Categories, _HasAlpha, _Debug, _IsCode);
    }

    // -----------------------------------------------------------------------------

    CShaderPtr CompileCS(const Base::Char* _pFileName, const Base::Char* _pShaderName, const Base::Char* _pShaderDefines, const Base::Char* _pShaderDescription, unsigned int _Categories, bool _HasAlpha, bool _Debug, bool _IsCode)
    {
        return CGfxShaderManager::GetInstance().CompileCS(_pFileName, _pShaderName, _pShaderDefines, _pShaderDescription, _Categories, _HasAlpha, _Debug, _IsCode);
    }

    // -----------------------------------------------------------------------------

    void ReloadShader(CShaderPtr _ShaderPtr)
    {
        CGfxShaderManager::GetInstance().ReloadShader(_ShaderPtr);
    }

    // -----------------------------------------------------------------------------

    void ReloadAllShaders()
    {
        return CGfxShaderManager::GetInstance().ReloadAllShaders();
    }

    // -----------------------------------------------------------------------------

    CInputLayoutPtr CreateInputLayout(const SInputElementDescriptor* _pDescriptors, unsigned int _NumberOfDescriptors, CShaderPtr _VertexShaderPtr)
    {
        return CGfxShaderManager::GetInstance().CreateInputLayout(_pDescriptors, _NumberOfDescriptors, _VertexShaderPtr);
    }

    // -----------------------------------------------------------------------------

    void SetShaderLabel(CShaderPtr _ShaderPtr, const char* _pLabel)
    {
        CGfxShaderManager::GetInstance().SetShaderLabel(_ShaderPtr, _pLabel);
    }
} // namespace ShaderManager
} // namespace Gfx
