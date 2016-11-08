//
//  gfx_material_manager.cpp
//  graphic
//
//  Created by Tobias Schwandt on 22/04/15.
//  Copyright (c) 2015 TU Ilmenau. All rights reserved.
//

#include "graphic/gfx_precompiled.h"

#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "graphic/gfx_material_manager.h"
#include "graphic/gfx_texture_manager.h"
#include "graphic/gfx_shader.h"
#include "graphic/gfx_shader_manager.h"

#include "tinyxml2.h"

using namespace Gfx;

// -----------------------------------------------------------------------------
// Here we define the some built-in shaders:
// -----------------------------------------------------------------------------
namespace
{
    // -----------------------------------------------------------------------------
    // Define shader combinations
    // -----------------------------------------------------------------------------
    static const unsigned int s_NoShader = static_cast<unsigned int>(-1);

    // -----------------------------------------------------------------------------
    // Define shader combinations
    // -----------------------------------------------------------------------------
    static const unsigned int s_NumberOfVertexShader = 3;

    // -----------------------------------------------------------------------------
    // Define all vertex shader needed inside this renderer
    // -----------------------------------------------------------------------------
    const Base::Char* g_pShaderFilenameVS[] =
    {
        "vs_m_pn.glsl",
        "vs_m_pnx0.glsl",
        "vs_m_pntbx0.glsl",
    };

    const Base::Char* g_pShaderNamesVS[] =
    {
        "VSShaderPN",
        "VSShaderPNX0",
        "VSShaderPNTBX0",
    };

    // -----------------------------------------------------------------------------
    // Define input layouts depending on vertex shader
    // -----------------------------------------------------------------------------
    struct SInputElementDescriptorSetting
    {
        unsigned int m_Offset;
        unsigned int m_NumberOfElements;
    };

    const SInputElementDescriptorSetting g_InputLayoutDescriptor[] =
    {
        { 0, 2 },
        { 2, 3 },
        { 5, 5 },
    };

    const Gfx::SInputElementDescriptor g_InputLayouts[] =
    {
        // VSShaderBlankPN
        { "POSITION", 0, Gfx::CInputLayout::Float3Format, 0,  0, 24, Gfx::CInputLayout::PerVertex, 0 },
        { "NORMAL"  , 0, Gfx::CInputLayout::Float3Format, 0, 12, 24, Gfx::CInputLayout::PerVertex, 0 },
        // VSShaderDiffusePNX0
        { "POSITION", 0, Gfx::CInputLayout::Float3Format, 0,  0, 32, Gfx::CInputLayout::PerVertex, 0 },
        { "NORMAL"  , 0, Gfx::CInputLayout::Float3Format, 0, 12, 32, Gfx::CInputLayout::PerVertex, 0 },
        { "TEXCOORD", 0, Gfx::CInputLayout::Float2Format, 0, 24, 32, Gfx::CInputLayout::PerVertex, 0 },
        // VSShaderBumpedPNX0
        { "POSITION" , 0, Gfx::CInputLayout::Float3Format, 0,  0, 56, Gfx::CInputLayout::PerVertex, 0 },
        { "NORMAL"   , 0, Gfx::CInputLayout::Float3Format, 0, 12, 56, Gfx::CInputLayout::PerVertex, 0 },
        { "TANGENT"  , 0, Gfx::CInputLayout::Float3Format, 0, 24, 56, Gfx::CInputLayout::PerVertex, 0 },
        { "BITANGENT", 0, Gfx::CInputLayout::Float3Format, 0, 36, 56, Gfx::CInputLayout::PerVertex, 0 },
        { "TEXCOORD" , 0, Gfx::CInputLayout::Float2Format, 0, 48, 56, Gfx::CInputLayout::PerVertex, 0 },
    };

    // -----------------------------------------------------------------------------
    // Define shader combinations
    // -----------------------------------------------------------------------------
    const Gfx::CSurface::SSurfaceKey g_SurfaceCombinations[s_NumberOfVertexShader] =
    {
        // -----------------------------------------------------------------------------
        // 01. Attribute: HasPosition
        // 02. Attribute: HasNormal
        // 04. Attribute: HasTangent
        // 05. Attribute: HasBitangent
        // 06. Attribute: HasTexCoords
        // -----------------------------------------------------------------------------

        // 01  , 02   , 03   , 04   , 05
        { true, true, false, false, false },
        { true, true, false, false, true  },
        { true, true, true,  true,  true  },
    };

    // -----------------------------------------------------------------------------
    // Define all hull shader needed inside this renderer
    // -----------------------------------------------------------------------------
    const Base::Char* g_pShaderFilenameHS[] =
    {
        "tcs_test.glsl",
    };

    const Base::Char* g_pShaderNamesHS[] =
    {
        "HSShader",
    };

    // -----------------------------------------------------------------------------
    // Define all domain shader needed inside this renderer
    // -----------------------------------------------------------------------------
    const Base::Char* g_pShaderFilenameDS[] =
    {
        "tes_test.glsl",
    };

    const Base::Char* g_pShaderNamesDS[] =
    {
        "DSShader",
    };

    // -----------------------------------------------------------------------------
    // Define all geometry pixel shader needed inside this renderer
    // -----------------------------------------------------------------------------
    const Base::Char* g_pShaderFilenameGS[] =
    {
        "gs_test.glsl",
    };

    const Base::Char* g_pShaderNamesGS[] =
    {
        "GSShader",
    };

    // -----------------------------------------------------------------------------
    // Define all pixel shader
    // -----------------------------------------------------------------------------
    const char* g_pShaderFilenamePS[] =
    {
        "fs_material.glsl",
        "fs_material.glsl",
        "fs_material.glsl",
        "fs_material.glsl",
        "fs_material.glsl",
        "fs_material.glsl",
        "fs_material.glsl",
        "fs_material.glsl",
    };

    const char* g_pShaderNamesPS[] =
    {
        "PSShaderMaterialDisneyBlank"  ,
        "PSShaderMaterialDisneyNX0"    ,
        "PSShaderMaterialDisneyCX0"    ,
        "PSShaderMaterialDisneyCNX0"   ,
        "PSShaderMaterialDisneyCNRX0"  ,
        "PSShaderMaterialDisneyCNRMX0" ,
        "PSShaderMaterialDisneyCNRMAX0",
        "PSShaderMaterialDisneyCNRRMX0",
    };

    // -----------------------------------------------------------------------------
    // Define shader combinations
    // -----------------------------------------------------------------------------
#pragma warning(disable:4201)
    struct SShaderShaderLink
    {
        union
        {
            struct
            {
                unsigned int m_pShaderNamesVS;
                unsigned int m_pShaderNamesHS;
                unsigned int m_pShaderNamesDS;
                unsigned int m_pShaderNamesGS;
                unsigned int m_pShaderNamesPS;
            };
            unsigned int m_ShaderType[5];
        };
    };
#pragma warning(default:4201)

    static const unsigned int s_NumberOfShaderMaterialCombinations = 10;

    const SShaderShaderLink g_ShaderShaderLinks[s_NumberOfShaderMaterialCombinations] =
    {
        { 0, s_NoShader, s_NoShader, s_NoShader, 0 },
        { 0, s_NoShader, s_NoShader, s_NoShader, 1 },
        { 0, s_NoShader, s_NoShader, s_NoShader, 2 },
        { 0, s_NoShader, s_NoShader, s_NoShader, 3 },
        { 0, s_NoShader, s_NoShader, s_NoShader, 4 },
        { 0, s_NoShader, s_NoShader, s_NoShader, 5 },
        { 0, s_NoShader, s_NoShader, s_NoShader, 6 },
        { 0, s_NoShader, s_NoShader, s_NoShader, 7 },
        { 0, 0         , 0         , s_NoShader, 2 },
        { 0, 0         , 0         , s_NoShader, 3 },
    };

    // -----------------------------------------------------------------------------
    // Material combinations
    // -----------------------------------------------------------------------------
    const Gfx::CMaterial::SMaterialKey g_MaterialCombinations[s_NumberOfShaderMaterialCombinations] =
    {
        // -----------------------------------------------------------------------------
        // 01. Attribute: HasDiffuseTex
        // 02. Attribute: HasNormalTex
        // 03. Attribute: HasRoughnessTex
        // 04. Attribute: HasReflectanceTex
        // 05. Attribute: HasMetallicTex
        // 06. Attribute: HasAOTex
        // 07. Attribute: HasBumpTex
        // -----------------------------------------------------------------------------

        // 01  , 02   , 03   , 04   , 05   , 06,    07
        { false, false, false, false, false, false, false },
        { false, true , false, false, false, false, false },
        { true , false, false, false, false, false, false },
        { true , true , false, false, false, false, false },
        { true , true , true , false, false, false, false },
        { true , true , true , false, true , false, false },
        { true , true , true , false, true , true , false },
        { true , true , true , true , true , false, false },
        { true , false, false, false, false, false, true  },
        { true , true , false, false, false, false, true  },
    };
} // namespace

namespace
{
    class CGfxMaterialManager : Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGfxMaterialManager);

    public:

        CGfxMaterialManager();
        ~CGfxMaterialManager();

    public:

        void OnStart();
        void OnExit();

        void Clear();

        CMaterialPtr CreateMaterial(const SMaterialDescriptor& _rDescriptor);

    private:

        class CInternMaterial : public CMaterial
        {
        private:

            friend class CGfxMaterialManager;
        };

    private:

        typedef Base::CManagedPool<CInternMaterial, 1024> CMaterials;

    private:

        CMaterials m_Materials;

    private:

        void OnNewMaterial(CSurface::SSurfaceKey::BSurfaceID _ID, CInternMaterial& _rMaterial) const;
    };
} // namespace

namespace
{
    CGfxMaterialManager::CGfxMaterialManager()
        : m_Materials()
    {

    }

    // -----------------------------------------------------------------------------

    CGfxMaterialManager::~CGfxMaterialManager()
    {

    }

    // -----------------------------------------------------------------------------

    void CGfxMaterialManager::OnStart()
    {

    }

    // -----------------------------------------------------------------------------

    void CGfxMaterialManager::OnExit()
    {
        Clear();
    }

    // -----------------------------------------------------------------------------

    void CGfxMaterialManager::Clear()
    {
        m_Materials.Clear();
    }

    // -----------------------------------------------------------------------------

    CMaterialPtr CGfxMaterialManager::CreateMaterial(const SMaterialDescriptor& _rDescriptor)
    {
        // -----------------------------------------------------------------------------
        // Get data material
        // -----------------------------------------------------------------------------
        const Dt::CMaterial& rDataMaterial = *_rDescriptor.m_pMaterial;
        
        // -----------------------------------------------------------------------------
        // Create material
        // -----------------------------------------------------------------------------
        CMaterials::CPtr MaterialPtr = m_Materials.Allocate();

        CInternMaterial& rMaterial = *MaterialPtr;

        // -----------------------------------------------------------------------------
        // Key estimation
        // -----------------------------------------------------------------------------
        rMaterial.m_MaterialKey.m_HasDiffuseTex     = rDataMaterial.m_pColorMap       != 0;
        rMaterial.m_MaterialKey.m_HasNormalTex      = rDataMaterial.m_pNormalMap      != 0;
        rMaterial.m_MaterialKey.m_HasRoughnessTex   = rDataMaterial.m_pRoughnessMap   != 0;
        rMaterial.m_MaterialKey.m_HasReflectanceTex = rDataMaterial.m_pReflectanceMap != 0;
        rMaterial.m_MaterialKey.m_HasMetallicTex    = rDataMaterial.m_pMetalMaskMap   != 0;
        rMaterial.m_MaterialKey.m_HasAOTex          = rDataMaterial.m_pAOMap          != 0;
        rMaterial.m_MaterialKey.m_HasBumpTex        = rDataMaterial.m_pBumpMap        != 0;

        // -----------------------------------------------------------------------------
        // Shader estimation depending on key
        // -----------------------------------------------------------------------------
        OnNewMaterial(_rDescriptor.m_ID, rMaterial);

        // -----------------------------------------------------------------------------
        // Set definitions
        // -----------------------------------------------------------------------------
        rMaterial.m_HasAlpha = false;
        rMaterial.m_HasBump  = rDataMaterial.m_pBumpMap != 0;

        // -----------------------------------------------------------------------------
        // Set attributes
        // -----------------------------------------------------------------------------
        rMaterial.m_MaterialAttributes.m_Color        = rDataMaterial.m_Color;
        rMaterial.m_MaterialAttributes.m_Roughness    = rDataMaterial.m_Roughness;
        rMaterial.m_MaterialAttributes.m_Reflectance  = rDataMaterial.m_Reflectance;
        rMaterial.m_MaterialAttributes.m_MetalMask    = rDataMaterial.m_MetalMask;
        rMaterial.m_MaterialAttributes.m_TilingOffset = rDataMaterial.m_TilingOffset;

        // -----------------------------------------------------------------------------
        // Create and setup shader, texture and sampler (setup material)
        // -----------------------------------------------------------------------------
        CTextureBasePtr    TexturePtrs[CMaterial::SMaterialKey::s_NumberOfTextures];
        STextureDescriptor TextureDescriptor;

        TextureDescriptor.m_NumberOfPixelsU  = STextureDescriptor::s_NumberOfPixelsFromSource;
        TextureDescriptor.m_NumberOfPixelsV  = STextureDescriptor::s_NumberOfPixelsFromSource;
        TextureDescriptor.m_NumberOfPixelsW  = STextureDescriptor::s_NumberOfPixelsFromSource;
        TextureDescriptor.m_NumberOfMipMaps  = STextureDescriptor::s_GenerateAllMipMaps;
        TextureDescriptor.m_NumberOfTextures = STextureDescriptor::s_NumberOfTexturesFromSource;
        TextureDescriptor.m_Binding          = CTextureBase::ShaderResource;
        TextureDescriptor.m_Access           = CTextureBase::CPUWrite;
        TextureDescriptor.m_Format           = CTextureBase::R8G8B8_UBYTE;
        TextureDescriptor.m_Usage            = CTextureBase::GPURead;
        TextureDescriptor.m_Semantic         = CTextureBase::Diffuse;
        TextureDescriptor.m_pPixels          = 0;
        TextureDescriptor.m_pFileName        = 0;

        TexturePtrs[0] = 0;
        TexturePtrs[1] = 0;
        TexturePtrs[2] = 0;
        TexturePtrs[3] = 0;
        TexturePtrs[4] = 0;
        TexturePtrs[5] = 0;
        TexturePtrs[6] = 0;

        if (rMaterial.m_MaterialKey.m_HasDiffuseTex)
        {
            TextureDescriptor.m_NumberOfPixelsU = rDataMaterial.m_pColorMap->GetNumberOfPixelsU();
            TextureDescriptor.m_NumberOfPixelsV = rDataMaterial.m_pColorMap->GetNumberOfPixelsV();
            TextureDescriptor.m_NumberOfPixelsW = 1;
            TextureDescriptor.m_pFileName       = rDataMaterial.m_pColorMap->GetFileName();
            TextureDescriptor.m_pPixels         = rDataMaterial.m_pColorMap->GetPixels();

            TexturePtrs[0]    = TextureManager::CreateTexture2D(TextureDescriptor);
        }

        if (rMaterial.m_MaterialKey.m_HasNormalTex)
        {
            TextureDescriptor.m_NumberOfPixelsU = rDataMaterial.m_pNormalMap->GetNumberOfPixelsU();
            TextureDescriptor.m_NumberOfPixelsV = rDataMaterial.m_pNormalMap->GetNumberOfPixelsV();
            TextureDescriptor.m_NumberOfPixelsW = 1;
            TextureDescriptor.m_pFileName       = rDataMaterial.m_pNormalMap->GetFileName();
            TextureDescriptor.m_pPixels         = rDataMaterial.m_pNormalMap->GetPixels();

            TexturePtrs[1]    = TextureManager::CreateTexture2D(TextureDescriptor);
        }

        if (rMaterial.m_MaterialKey.m_HasRoughnessTex)
        {
            TextureDescriptor.m_NumberOfPixelsU = rDataMaterial.m_pRoughnessMap->GetNumberOfPixelsU();
            TextureDescriptor.m_NumberOfPixelsV = rDataMaterial.m_pRoughnessMap->GetNumberOfPixelsV();
            TextureDescriptor.m_NumberOfPixelsW = 1;
            TextureDescriptor.m_pFileName       = rDataMaterial.m_pRoughnessMap->GetFileName();
            TextureDescriptor.m_pPixels         = rDataMaterial.m_pRoughnessMap->GetPixels();

            TexturePtrs[2] = TextureManager::CreateTexture2D(TextureDescriptor);
        }

        if (rMaterial.m_MaterialKey.m_HasReflectanceTex)
        {
            TextureDescriptor.m_NumberOfPixelsU = rDataMaterial.m_pReflectanceMap->GetNumberOfPixelsU();
            TextureDescriptor.m_NumberOfPixelsV = rDataMaterial.m_pReflectanceMap->GetNumberOfPixelsV();
            TextureDescriptor.m_NumberOfPixelsW = 1;
            TextureDescriptor.m_pFileName       = rDataMaterial.m_pReflectanceMap->GetFileName();
            TextureDescriptor.m_pPixels         = rDataMaterial.m_pReflectanceMap->GetPixels();

            TexturePtrs[3] = TextureManager::CreateTexture2D(TextureDescriptor);
        }

        if (rMaterial.m_MaterialKey.m_HasMetallicTex)
        {
            TextureDescriptor.m_NumberOfPixelsU = rDataMaterial.m_pMetalMaskMap->GetNumberOfPixelsU();
            TextureDescriptor.m_NumberOfPixelsV = rDataMaterial.m_pMetalMaskMap->GetNumberOfPixelsV();
            TextureDescriptor.m_NumberOfPixelsW = 1;
            TextureDescriptor.m_pFileName       = rDataMaterial.m_pMetalMaskMap->GetFileName();
            TextureDescriptor.m_pPixels         = rDataMaterial.m_pMetalMaskMap->GetPixels();

            TexturePtrs[4] = TextureManager::CreateTexture2D(TextureDescriptor);
        }

        if (rMaterial.m_MaterialKey.m_HasAOTex)
        {
            TextureDescriptor.m_NumberOfPixelsU = rDataMaterial.m_pAOMap->GetNumberOfPixelsU();
            TextureDescriptor.m_NumberOfPixelsV = rDataMaterial.m_pAOMap->GetNumberOfPixelsV();
            TextureDescriptor.m_NumberOfPixelsW = 1;
            TextureDescriptor.m_pFileName       = rDataMaterial.m_pAOMap->GetFileName();
            TextureDescriptor.m_pPixels         = rDataMaterial.m_pAOMap->GetPixels();

            TexturePtrs[5] = TextureManager::CreateTexture2D(TextureDescriptor);
        }

        if (rMaterial.m_MaterialKey.m_HasBumpTex)
        {
            TextureDescriptor.m_NumberOfPixelsU = rDataMaterial.m_pBumpMap->GetNumberOfPixelsU();
            TextureDescriptor.m_NumberOfPixelsV = rDataMaterial.m_pBumpMap->GetNumberOfPixelsV();
            TextureDescriptor.m_NumberOfPixelsW = 1;
            TextureDescriptor.m_Format          = CTextureBase::R8_UBYTE;
            TextureDescriptor.m_pFileName       = rDataMaterial.m_pBumpMap->GetFileName();
            TextureDescriptor.m_pPixels         = rDataMaterial.m_pBumpMap->GetPixels();

            TexturePtrs[6] = TextureManager::CreateTexture2D(TextureDescriptor);
        }

        rMaterial.m_TextureSetPtrs[CShader::Pixel] = TextureManager::CreateTextureSet(TexturePtrs, CMaterial::SMaterialKey::s_NumberOfTextures);

        return CMaterialPtr(MaterialPtr);
    }

    // -----------------------------------------------------------------------------

    void CGfxMaterialManager::OnNewMaterial(CSurface::SSurfaceKey::BSurfaceID _ID, CInternMaterial& _rMaterial) const
    {
        unsigned int ShaderLinkIndex = 0;

        // -----------------------------------------------------------------------------
        // Try to find the right shader for that surface
        // If no shader was found we use a blank shader with pink color.
        // -----------------------------------------------------------------------------
        for (unsigned int IndexOfShader = 0; IndexOfShader < s_NumberOfVertexShader; ++IndexOfShader)
        {
            Gfx::CSurface::SSurfaceKey::BSurfaceID TempMostReliableKey = g_SurfaceCombinations[IndexOfShader].m_Key & _ID;

            if (g_SurfaceCombinations[IndexOfShader].m_Key == TempMostReliableKey)
            {
                ShaderLinkIndex = IndexOfShader;
            }
        }

        // -----------------------------------------------------------------------------
        // Now we get the index of the real vertex and pixel shader
        // -----------------------------------------------------------------------------
        const unsigned int VSIndex = ShaderLinkIndex;

        // -----------------------------------------------------------------------------
        // Compile shader
        // -----------------------------------------------------------------------------
        Gfx::CShaderPtr VSShader = Gfx::ShaderManager::CompileVS(g_pShaderFilenameVS[VSIndex], g_pShaderNamesVS[VSIndex]);

        assert(VSShader != 0);

        // -----------------------------------------------------------------------------
        // Set input layout if vertex shader has no input layout
        // -----------------------------------------------------------------------------
        if (VSShader->GetInputLayout() == nullptr)
        {
            SInputElementDescriptorSetting InputLayoutDesc = g_InputLayoutDescriptor[VSIndex];

            Gfx::SInputElementDescriptor* pDescriptor = static_cast<Gfx::SInputElementDescriptor*>(Base::CMemory::Allocate(sizeof(Gfx::SInputElementDescriptor) * InputLayoutDesc.m_NumberOfElements));

            unsigned int IndexOfRenderInputDesc = InputLayoutDesc.m_Offset;

            for (unsigned int IndexOfElement = 0; IndexOfElement < InputLayoutDesc.m_NumberOfElements; ++IndexOfElement)
            {
                pDescriptor[IndexOfElement] = g_InputLayouts[IndexOfRenderInputDesc];

                ++IndexOfRenderInputDesc;
            }

            Gfx::CInputLayoutPtr LayoutPtr = Gfx::ShaderManager::CreateInputLayout(pDescriptor, InputLayoutDesc.m_NumberOfElements, VSShader);

            Base::CMemory::Free(pDescriptor);
        }

        _rMaterial.m_ShaderPtrs[CShader::Vertex] = VSShader;

        // -----------------------------------------------------------------------------
        // Try to find the right shader for that surface
        // If no shader was found we use a blank shader with pink color.
        // -----------------------------------------------------------------------------
        const Gfx::CMaterial::SMaterialKey & rModelAttributes = _rMaterial.m_MaterialKey;
        Gfx::CMaterial::SMaterialKey::BMaterialID MostReliableKey = 0;

        for (unsigned int IndexOfShader = 0; IndexOfShader < s_NumberOfShaderMaterialCombinations; ++IndexOfShader)
        {
            Gfx::CMaterial::SMaterialKey::BMaterialID TempMostReliableKey = g_MaterialCombinations[IndexOfShader].m_Key & rModelAttributes.m_Key;

            if (g_MaterialCombinations[IndexOfShader].m_Key == TempMostReliableKey)
            {
                ShaderLinkIndex = IndexOfShader;
                MostReliableKey = TempMostReliableKey;
            }
        }

        // -----------------------------------------------------------------------------
        // Reset key
        // -----------------------------------------------------------------------------
        _rMaterial.m_MaterialKey.m_Key = MostReliableKey;

        // -----------------------------------------------------------------------------
        // Now we get the names of the pixel shader
        // -----------------------------------------------------------------------------
        const unsigned int HSIndex = g_ShaderShaderLinks[ShaderLinkIndex].m_pShaderNamesHS;
        const unsigned int DSIndex = g_ShaderShaderLinks[ShaderLinkIndex].m_pShaderNamesDS;
        const unsigned int GSIndex = g_ShaderShaderLinks[ShaderLinkIndex].m_pShaderNamesGS;
        const unsigned int PSIndex = g_ShaderShaderLinks[ShaderLinkIndex].m_pShaderNamesPS;

        // -----------------------------------------------------------------------------
        // Compile shader
        // -----------------------------------------------------------------------------
        if (HSIndex != s_NoShader)
        {
            _rMaterial.m_ShaderPtrs[CShader::Hull] = Gfx::ShaderManager::CompileHS(g_pShaderFilenameHS[HSIndex], g_pShaderNamesHS[HSIndex]);
        }

        if (DSIndex != s_NoShader)
        {
            _rMaterial.m_ShaderPtrs[CShader::Domain] = Gfx::ShaderManager::CompileDS(g_pShaderFilenameDS[DSIndex], g_pShaderNamesDS[DSIndex]);
        }

        if (GSIndex != s_NoShader)
        {
            _rMaterial.m_ShaderPtrs[CShader::Geometry] = Gfx::ShaderManager::CompileGS(g_pShaderFilenameGS[GSIndex], g_pShaderNamesGS[GSIndex]);
        }

        assert(PSIndex != s_NoShader);

        if (PSIndex != s_NoShader)
        {
            const Base::Char* pShaderfileGb = g_pShaderFilenamePS[PSIndex];
            const Base::Char* pShadernameGb = g_pShaderNamesPS[PSIndex];

            _rMaterial.m_ShaderPtrs[CShader::Pixel] = Gfx::ShaderManager::CompilePS(pShaderfileGb, pShadernameGb);
        }
    }
} // namespace

namespace Gfx
{
namespace MaterialManager
{
    void OnStart()
    {
        CGfxMaterialManager::GetInstance().OnStart();
    }

    // -----------------------------------------------------------------------------

    void OnExit()
    {
        CGfxMaterialManager::GetInstance().OnExit();
    }

    // -----------------------------------------------------------------------------

    void Clear()
    {
        CGfxMaterialManager::GetInstance().Clear();
    }

    // -----------------------------------------------------------------------------

    CMaterialPtr CreateMaterial(const SMaterialDescriptor& _rDescriptor)
    {
        return CGfxMaterialManager::GetInstance().CreateMaterial(_rDescriptor);
    }
} // namespace MaterialManager
} // namespace Gfx
