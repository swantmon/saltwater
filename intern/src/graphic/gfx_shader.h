
#pragma once

#include "base/base_managed_pool.h"
#include "base/base_typedef.h"

#include "graphic/gfx_buffer_set.h"
#include "graphic/gfx_export.h"
#include "graphic/gfx_input_layout.h"

#include <string>
#include <vector>

namespace Gfx
{
    class CShader : public Base::CManagedPoolItemBase
    {
    public:
    
        // -----------------------------------------------------------------------------
        // Shader Pipeline (* = fixed):
        //
        // OpenGL:                              | DirectX:
        //                                      |
        // -> Vertex                            | -> Vertex
        //  -> Tesselation Control              |  -> Hull
        //   -> Tesselator(*)                   |   -> Tesselator(*)
        //    -> Tesselation Evaluation         |    -> Domain
        //     -> Geometry                      |     -> Geometry
        //      -> Rasterizer(*)                |      -> Rasterizer(*)
        //       -> Fragment                    |       -> Pixel
        //
        // -----------------------------------------------------------------------------
    
        enum EType
        {
            Vertex,
            Hull,
            Domain,
            Geometry,
            Pixel,
            Compute,
            NumberOfTypes,
            UndefinedType = -1,
        };
        
    public:

        static const unsigned int s_MaxNumberOfConstantBufferInfos = 16;
        static const unsigned int s_InvalidID                      = static_cast<unsigned int>(-1);

    public:

        typedef std::vector<Base::CManagedPoolItemPtr<CShader>>::iterator CShaderLinkIterator;

    public:

        GFX_API unsigned int GetID() const;
        GFX_API EType GetType() const;
        GFX_API const std::string& GetFileName() const;
        GFX_API const std::string& GetShaderName() const;
        GFX_API const std::string& GetShaderDefines() const;

    public:

        GFX_API bool HasAlpha() const;

    public:

        GFX_API CInputLayoutPtr GetInputLayout();

    protected:

        typedef std::vector<Base::CManagedPoolItemPtr<CShader>> CLinkVector;

    protected:

        unsigned int    m_ID;
        bool            m_HasAlpha;
        CInputLayoutPtr m_InputLayoutPtr;
        std::string     m_FileName;           ///< The name of the file containing the shader function.
        std::string     m_ShaderName;         ///< The name of the shader function in the file.
        std::string     m_ShaderDefines;      ///< The defines of the shader in the file.
        EType           m_Type;                ///< Identifies the shader stage.

    protected:

        CShader();
       ~CShader();
    };
} // namespace Gfx

namespace Gfx
{
    typedef Base::CManagedPoolItemPtr<CShader> CShaderPtr;
} // namespace Gfx
