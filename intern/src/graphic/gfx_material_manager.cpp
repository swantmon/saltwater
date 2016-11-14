//
//  gfx_material_manager.cpp
//  graphic
//
//  Created by Tobias Schwandt on 22/04/15.
//  Copyright (c) 2015 TU Ilmenau. All rights reserved.
//

#include "graphic/gfx_precompiled.h"

#include "base/base_exception.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "data/data_material_manager.h"

#include "graphic/gfx_material_manager.h"
#include "graphic/gfx_texture_manager.h"
#include "graphic/gfx_shader.h"
#include "graphic/gfx_shader_manager.h"

#include "tinyxml2.h"

#include <unordered_map>

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
    std::string g_PathToAssets = "../assets/";
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

        CMaterialPtr CreateMaterial(const SMaterialDescriptor& _rDescriptor);

        CMaterialPtr GetDefaultMaterialByHash();

        CMaterialPtr GetMaterialByHash(unsigned int _Hash);

    private:

        class CInternMaterial : public CMaterial
        {
        private:

            friend class CGfxMaterialManager;
        };

    private:

        typedef Base::CManagedPool<CInternMaterial, 1024> CMaterials;

        typedef std::unordered_map<unsigned int, CInternMaterial*> CMaterialByHashs;

    private:

        CMaterials   m_Materials;
        CMaterialPtr m_DefaultMaterialPtr;

        CMaterialByHashs m_MaterialByHash;

    private:

        void OnDirtyMaterial(Dt::CMaterial* _Material);

        CInternMaterial* InternCreateMaterial(const SMaterialDescriptor& _rDescriptor);

        void OnNewMaterial(CInternMaterial& _rMaterial) const;
    };
} // namespace

namespace
{
    CGfxMaterialManager::CGfxMaterialManager()
        : m_Materials         ()
        , m_DefaultMaterialPtr()
        , m_MaterialByHash    ()
    {

    }

    // -----------------------------------------------------------------------------

    CGfxMaterialManager::~CGfxMaterialManager()
    {

    }

    // -----------------------------------------------------------------------------

    void CGfxMaterialManager::OnStart()
    {
        // -----------------------------------------------------------------------------
        // Create default material
        // -----------------------------------------------------------------------------
        m_DefaultMaterialPtr = m_Materials.Allocate();

        // -----------------------------------------------------------------------------
        // Set dirty handler of data textures
        // -----------------------------------------------------------------------------
        Dt::MaterialManager::RegisterDirtyMaterialHandler(DATA_DIRTY_MATERIAL_METHOD(&CGfxMaterialManager::OnDirtyMaterial));
    }

    // -----------------------------------------------------------------------------

    void CGfxMaterialManager::OnExit()
    {
        m_Materials.Clear();

        m_MaterialByHash.clear();
    }

    // -----------------------------------------------------------------------------

    CMaterialPtr CGfxMaterialManager::CreateMaterial(const SMaterialDescriptor& _rDescriptor)
    {
        CInternMaterial* pInternMaterial = InternCreateMaterial(_rDescriptor);

        if (pInternMaterial == nullptr)
        {
            return m_DefaultMaterialPtr;
        }

        return CMaterialPtr(pInternMaterial);
    }

    // -----------------------------------------------------------------------------

    CMaterialPtr CGfxMaterialManager::GetDefaultMaterialByHash()
    {
        return m_DefaultMaterialPtr;
    }

    // -----------------------------------------------------------------------------

    CMaterialPtr CGfxMaterialManager::GetMaterialByHash(unsigned int _Hash)
    {
        if (m_MaterialByHash.find(_Hash) != m_MaterialByHash.end())
        {
            return m_MaterialByHash.at(_Hash);
        }

        return nullptr;
    }

    // -----------------------------------------------------------------------------

    void CGfxMaterialManager::OnDirtyMaterial(Dt::CMaterial* _Material)
    {
        if (_Material == nullptr) return;

        unsigned int DirtyFlags = _Material->GetDirtyFlags();
        unsigned int Hash       = _Material->GetHash();

        if ((DirtyFlags & Dt::CTextureBase::DirtyCreate) != 0)
        {
        }

        if ((DirtyFlags & Dt::CTextureBase::DirtyData) != 0)
        {
            // -----------------------------------------------------------------------------
            // Get data material
            // -----------------------------------------------------------------------------
            Dt::CMaterial&   rDataMaterial    = *_Material;
            CInternMaterial* pGraphicMaterial = m_MaterialByHash.at(Hash);

            CInternMaterial& rMaterial = *pGraphicMaterial;

            // -----------------------------------------------------------------------------
            // Key estimation
            // -----------------------------------------------------------------------------
            rMaterial.m_MaterialKey.m_HasDiffuseTex     = rDataMaterial.GetColorTexture()            != 0;
            rMaterial.m_MaterialKey.m_HasNormalTex      = rDataMaterial.GetNormalTexture()           != 0;
            rMaterial.m_MaterialKey.m_HasRoughnessTex   = rDataMaterial.GetRoughnessTexture()        != 0;
            rMaterial.m_MaterialKey.m_HasReflectanceTex = rDataMaterial.GetReflectanceTexture()      != 0;
            rMaterial.m_MaterialKey.m_HasMetallicTex    = rDataMaterial.GetMetalTexture()            != 0;
            rMaterial.m_MaterialKey.m_HasAOTex          = rDataMaterial.GetAmbientOcclusionTexture() != 0;
            rMaterial.m_MaterialKey.m_HasBumpTex        = rDataMaterial.GetBumpTexture()             != 0;

            // -----------------------------------------------------------------------------
            // Shader estimation depending on key
            // -----------------------------------------------------------------------------
            OnNewMaterial(rMaterial);

            // -----------------------------------------------------------------------------
            // Set definitions
            // -----------------------------------------------------------------------------
            rMaterial.m_HasAlpha = false;
            rMaterial.m_HasBump  = rDataMaterial.GetBumpTexture() != 0;

            // -----------------------------------------------------------------------------
            // Set attributes
            // -----------------------------------------------------------------------------
            rMaterial.m_MaterialAttributes.m_Color        = rDataMaterial.GetColor();
            rMaterial.m_MaterialAttributes.m_Roughness    = rDataMaterial.GetRoughness();
            rMaterial.m_MaterialAttributes.m_Reflectance  = rDataMaterial.GetReflectance();
            rMaterial.m_MaterialAttributes.m_MetalMask    = rDataMaterial.GetMetalness();
            rMaterial.m_MaterialAttributes.m_TilingOffset = rDataMaterial.GetTilingOffset();

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
                TextureDescriptor.m_NumberOfPixelsU = rDataMaterial.GetColorTexture()->GetNumberOfPixelsU();
                TextureDescriptor.m_NumberOfPixelsV = rDataMaterial.GetColorTexture()->GetNumberOfPixelsV();
                TextureDescriptor.m_NumberOfPixelsW = 1;
                TextureDescriptor.m_pFileName       = rDataMaterial.GetColorTexture()->GetFileName();
                TextureDescriptor.m_pPixels         = rDataMaterial.GetColorTexture()->GetPixels();

                TexturePtrs[0]    = TextureManager::CreateTexture2D(TextureDescriptor);
            }

            if (rMaterial.m_MaterialKey.m_HasNormalTex)
            {
                TextureDescriptor.m_NumberOfPixelsU = rDataMaterial.GetNormalTexture()->GetNumberOfPixelsU();
                TextureDescriptor.m_NumberOfPixelsV = rDataMaterial.GetNormalTexture()->GetNumberOfPixelsV();
                TextureDescriptor.m_NumberOfPixelsW = 1;
                TextureDescriptor.m_pFileName       = rDataMaterial.GetNormalTexture()->GetFileName();
                TextureDescriptor.m_pPixels         = rDataMaterial.GetNormalTexture()->GetPixels();

                TexturePtrs[1]    = TextureManager::CreateTexture2D(TextureDescriptor);
            }

            if (rMaterial.m_MaterialKey.m_HasRoughnessTex)
            {
                TextureDescriptor.m_NumberOfPixelsU = rDataMaterial.GetRoughnessTexture()->GetNumberOfPixelsU();
                TextureDescriptor.m_NumberOfPixelsV = rDataMaterial.GetRoughnessTexture()->GetNumberOfPixelsV();
                TextureDescriptor.m_NumberOfPixelsW = 1;
                TextureDescriptor.m_pFileName       = rDataMaterial.GetRoughnessTexture()->GetFileName();
                TextureDescriptor.m_pPixels         = rDataMaterial.GetRoughnessTexture()->GetPixels();

                TexturePtrs[2] = TextureManager::CreateTexture2D(TextureDescriptor);
            }

            if (rMaterial.m_MaterialKey.m_HasReflectanceTex)
            {
                TextureDescriptor.m_NumberOfPixelsU = rDataMaterial.GetReflectanceTexture()->GetNumberOfPixelsU();
                TextureDescriptor.m_NumberOfPixelsV = rDataMaterial.GetReflectanceTexture()->GetNumberOfPixelsV();
                TextureDescriptor.m_NumberOfPixelsW = 1;
                TextureDescriptor.m_pFileName       = rDataMaterial.GetReflectanceTexture()->GetFileName();
                TextureDescriptor.m_pPixels         = rDataMaterial.GetReflectanceTexture()->GetPixels();

                TexturePtrs[3] = TextureManager::CreateTexture2D(TextureDescriptor);
            }

            if (rMaterial.m_MaterialKey.m_HasMetallicTex)
            {
                TextureDescriptor.m_NumberOfPixelsU = rDataMaterial.GetMetalTexture()->GetNumberOfPixelsU();
                TextureDescriptor.m_NumberOfPixelsV = rDataMaterial.GetMetalTexture()->GetNumberOfPixelsV();
                TextureDescriptor.m_NumberOfPixelsW = 1;
                TextureDescriptor.m_pFileName       = rDataMaterial.GetMetalTexture()->GetFileName();
                TextureDescriptor.m_pPixels         = rDataMaterial.GetMetalTexture()->GetPixels();

                TexturePtrs[4] = TextureManager::CreateTexture2D(TextureDescriptor);
            }

            if (rMaterial.m_MaterialKey.m_HasAOTex)
            {
                TextureDescriptor.m_NumberOfPixelsU = rDataMaterial.GetAmbientOcclusionTexture()->GetNumberOfPixelsU();
                TextureDescriptor.m_NumberOfPixelsV = rDataMaterial.GetAmbientOcclusionTexture()->GetNumberOfPixelsV();
                TextureDescriptor.m_NumberOfPixelsW = 1;
                TextureDescriptor.m_pFileName       = rDataMaterial.GetAmbientOcclusionTexture()->GetFileName();
                TextureDescriptor.m_pPixels         = rDataMaterial.GetAmbientOcclusionTexture()->GetPixels();

                TexturePtrs[5] = TextureManager::CreateTexture2D(TextureDescriptor);
            }

            if (rMaterial.m_MaterialKey.m_HasBumpTex)
            {
                TextureDescriptor.m_NumberOfPixelsU = rDataMaterial.GetBumpTexture()->GetNumberOfPixelsU();
                TextureDescriptor.m_NumberOfPixelsV = rDataMaterial.GetBumpTexture()->GetNumberOfPixelsV();
                TextureDescriptor.m_NumberOfPixelsW = 1;
                TextureDescriptor.m_Format          = CTextureBase::R8_UBYTE;
                TextureDescriptor.m_pFileName       = rDataMaterial.GetBumpTexture()->GetFileName();
                TextureDescriptor.m_pPixels         = rDataMaterial.GetBumpTexture()->GetPixels();

                TexturePtrs[6] = TextureManager::CreateTexture2D(TextureDescriptor);
            }

            rMaterial.m_TextureSetPtrs[CShader::Pixel] = TextureManager::CreateTextureSet(TexturePtrs, CMaterial::SMaterialKey::s_NumberOfTextures);

            if (Hash != 0)
            {
                m_MaterialByHash[Hash] = pGraphicMaterial;
            }
        }
    }

    // -----------------------------------------------------------------------------

    CGfxMaterialManager::CInternMaterial* CGfxMaterialManager::InternCreateMaterial(const SMaterialDescriptor& _rDescriptor)
    {
        const char*  pMaterialName;
        const char*  pColorMap;
        const char*  pNormalMap;
        const char*  pRoughnessMap;
        const char*  pReflectanceMap;
        const char*  pMetalMaskMap;
        const char*  pAOMap;
        const char*  pBumpMap;
        float        Roughness;
        float        Reflectance;
        float        MetalMask;
        Base::Float3 AlbedoColor;
        Base::Float4 TilingOffset;

        pMaterialName   = _rDescriptor.m_pMaterialName;
        pColorMap       = _rDescriptor.m_pColorMap;
        pNormalMap      = _rDescriptor.m_pNormalMap;
        pRoughnessMap   = _rDescriptor.m_pRoughnessMap;
        pReflectanceMap = _rDescriptor.m_pReflectanceMap;
        pMetalMaskMap   = _rDescriptor.m_pMetalMaskMap;
        pAOMap          = _rDescriptor.m_pAOMap;
        pBumpMap        = _rDescriptor.m_pBumpMap;
        Roughness       = _rDescriptor.m_Roughness;
        Reflectance     = _rDescriptor.m_Reflectance;
        MetalMask       = _rDescriptor.m_MetalMask;
        AlbedoColor     = _rDescriptor.m_AlbedoColor;
        TilingOffset    = _rDescriptor.m_TilingOffset;

        if (_rDescriptor.m_pFileName != nullptr)
        {
            // -----------------------------------------------------------------------------
            // Build path to texture in file system
            // -----------------------------------------------------------------------------
		    std::string PathToMaterial = g_PathToAssets + _rDescriptor.m_pFileName;
        
            // -----------------------------------------------------------------------------
            // Load material file
            // -----------------------------------------------------------------------------
            tinyxml2::XMLDocument MaterialFile;
        
            int Error = MaterialFile.LoadFile(PathToMaterial.c_str());

            if (Error != tinyxml2::XML_NO_ERROR)
            {
                BASE_THROWV("Error loading material file '%s'.", PathToMaterial.c_str());
            }
        
            tinyxml2::XMLElement* pMaterialDefinition = MaterialFile.FirstChildElement("MaterialDefinition");
        
            // -----------------------------------------------------------------------------
            // Pull general informations from file
            // -----------------------------------------------------------------------------
            pMaterialName = pMaterialDefinition->Attribute("Name");
        
            assert(pMaterialName != 0);
        
            // -----------------------------------------------------------------------------
            // Color
            // -----------------------------------------------------------------------------
            tinyxml2::XMLElement* pMaterialColor = pMaterialDefinition->FirstChildElement("Color");
        
            assert(pMaterialColor != 0);
        
            float ColorR = pMaterialColor->FloatAttribute("R");
            float ColorG = pMaterialColor->FloatAttribute("G");
            float ColorB = pMaterialColor->FloatAttribute("B");

            AlbedoColor = Base::Float3(ColorR, ColorG, ColorB);
        
            pColorMap = pMaterialColor->Attribute("Map");
        
            // -----------------------------------------------------------------------------
            // Normal
            // -----------------------------------------------------------------------------
            tinyxml2::XMLElement* pMaterialNormal = pMaterialDefinition->FirstChildElement("Normal");
        
            assert(pMaterialNormal != 0);
        
            pNormalMap = pMaterialNormal->Attribute("Map");
        
            // -----------------------------------------------------------------------------
            // Roughness
            // -----------------------------------------------------------------------------
            tinyxml2::XMLElement* pMaterialRoughness = pMaterialDefinition->FirstChildElement("Roughness");
        
            assert(pMaterialRoughness != 0);
        
            Roughness = pMaterialRoughness->FloatAttribute("V");
        
            pRoughnessMap = pMaterialRoughness->Attribute("Map");
        
            // -----------------------------------------------------------------------------
            // Reflectance
            // -----------------------------------------------------------------------------
            tinyxml2::XMLElement* pMaterialReflectance = pMaterialDefinition->FirstChildElement("Reflectance");
        
            assert(pMaterialReflectance != 0);
        
            Reflectance = pMaterialReflectance->FloatAttribute("V");
        
            pReflectanceMap = pMaterialReflectance->Attribute("Map");
        
            // -----------------------------------------------------------------------------
            // Metallic
            // -----------------------------------------------------------------------------
            tinyxml2::XMLElement* pMaterialMetallic = pMaterialDefinition->FirstChildElement("Metallic");
        
            assert(pMaterialMetallic != 0);
        
            MetalMask = pMaterialMetallic->FloatAttribute("V");
        
            pMetalMaskMap = pMaterialMetallic->Attribute("Map");
        
            // -----------------------------------------------------------------------------
            // Ambient Occlusion
            // -----------------------------------------------------------------------------
            tinyxml2::XMLElement* pAO = pMaterialDefinition->FirstChildElement("AO");
        
            pAOMap = 0;
        
            if (pAO != 0)
            {
                pAOMap = pAO->Attribute("Map");
            }
        
            // -----------------------------------------------------------------------------
            // Bump
            // -----------------------------------------------------------------------------
            tinyxml2::XMLElement* pBump = pMaterialDefinition->FirstChildElement("Bump");
        
            pBumpMap = 0;
        
            if (pBump != 0)
            {
                pBumpMap = pBump->Attribute("Map");
            }
        
            // -----------------------------------------------------------------------------
            // Tiling & offset
            // -----------------------------------------------------------------------------
            tinyxml2::XMLElement* pTiling = pMaterialDefinition->FirstChildElement("Tiling");
            tinyxml2::XMLElement* pOffset = pMaterialDefinition->FirstChildElement("Offset");
        
            if (pTiling)
            {
                TilingOffset[0] = pTiling->FloatAttribute("X");
                TilingOffset[1] = pTiling->FloatAttribute("Y");
            }
        
            if (pOffset)
            {
                TilingOffset[2] = pOffset->FloatAttribute("X");
                TilingOffset[3] = pOffset->FloatAttribute("Y");
            }
        }

        // -----------------------------------------------------------------------------
        // Create material
        // -----------------------------------------------------------------------------
        CInternMaterial* pInternMaterial = m_Materials.Allocate();
        
        assert(pInternMaterial != nullptr);

        CInternMaterial& rMaterial = *pInternMaterial;

        // -----------------------------------------------------------------------------
        // Key estimation
        // -----------------------------------------------------------------------------
        rMaterial.m_MaterialKey.m_HasDiffuseTex     = pColorMap     != 0;
        rMaterial.m_MaterialKey.m_HasNormalTex      = pNormalMap    != 0;
        rMaterial.m_MaterialKey.m_HasRoughnessTex   = pRoughnessMap != 0;
        rMaterial.m_MaterialKey.m_HasReflectanceTex = 0;
        rMaterial.m_MaterialKey.m_HasMetallicTex    = pMetalMaskMap != 0;
        rMaterial.m_MaterialKey.m_HasAOTex          = pAOMap        != 0;
        rMaterial.m_MaterialKey.m_HasBumpTex        = pBumpMap      != 0;

        // -----------------------------------------------------------------------------
        // Shader estimation depending on key
        // -----------------------------------------------------------------------------
        OnNewMaterial(rMaterial);

        // -----------------------------------------------------------------------------
        // Set definitions
        // -----------------------------------------------------------------------------
        rMaterial.m_HasAlpha = false;
        rMaterial.m_HasBump  = pBumpMap != 0;

        // -----------------------------------------------------------------------------
        // Set attributes
        // -----------------------------------------------------------------------------
        rMaterial.m_MaterialAttributes.m_Color        = AlbedoColor;
        rMaterial.m_MaterialAttributes.m_Roughness    = Roughness;
        rMaterial.m_MaterialAttributes.m_Reflectance  = Reflectance;
        rMaterial.m_MaterialAttributes.m_MetalMask    = MetalMask;
        rMaterial.m_MaterialAttributes.m_TilingOffset = TilingOffset;

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
            TextureDescriptor.m_pFileName = pColorMap;

            TexturePtrs[0] = TextureManager::CreateTexture2D(TextureDescriptor);
        }

        if (rMaterial.m_MaterialKey.m_HasNormalTex)
        {
            TextureDescriptor.m_pFileName = pNormalMap;

            TexturePtrs[1] = TextureManager::CreateTexture2D(TextureDescriptor);
        }

        if (rMaterial.m_MaterialKey.m_HasRoughnessTex)
        {
            TextureDescriptor.m_pFileName = pRoughnessMap;

            TexturePtrs[2] = TextureManager::CreateTexture2D(TextureDescriptor);
        }

        if (rMaterial.m_MaterialKey.m_HasReflectanceTex)
        {
            TextureDescriptor.m_pFileName = pReflectanceMap;

            TexturePtrs[3] = TextureManager::CreateTexture2D(TextureDescriptor);
        }

        if (rMaterial.m_MaterialKey.m_HasMetallicTex)
        {
            TextureDescriptor.m_pFileName = pMetalMaskMap;

            TexturePtrs[4] = TextureManager::CreateTexture2D(TextureDescriptor);
        }

        if (rMaterial.m_MaterialKey.m_HasAOTex)
        {
            TextureDescriptor.m_pFileName = pAOMap;

            TexturePtrs[5] = TextureManager::CreateTexture2D(TextureDescriptor);
        }

        if (rMaterial.m_MaterialKey.m_HasBumpTex)
        {
            TextureDescriptor.m_NumberOfPixelsW = 1;
            TextureDescriptor.m_Format          = CTextureBase::R8_UBYTE;
            TextureDescriptor.m_pFileName       = pBumpMap;

            TexturePtrs[6] = TextureManager::CreateTexture2D(TextureDescriptor);
        }

        rMaterial.m_TextureSetPtrs[CShader::Pixel] = TextureManager::CreateTextureSet(TexturePtrs, CMaterial::SMaterialKey::s_NumberOfTextures);

        return pInternMaterial;
    }

    // -----------------------------------------------------------------------------

    void CGfxMaterialManager::OnNewMaterial(CInternMaterial& _rMaterial) const
    {
        unsigned int ShaderLinkIndex = 0;

        // -----------------------------------------------------------------------------
        // Try to find the right shader for that surface
        // If no shader was found we use a blank shader with pink color.
        // -----------------------------------------------------------------------------
        for (unsigned int IndexOfShader = 0; IndexOfShader < s_NumberOfVertexShader; ++IndexOfShader)
        {
            Gfx::CSurface::SSurfaceKey::BSurfaceID TempMostReliableKey = g_SurfaceCombinations[IndexOfShader].m_Key;

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

    CMaterialPtr CreateMaterial(const SMaterialDescriptor& _rDescriptor)
    {
        return CGfxMaterialManager::GetInstance().CreateMaterial(_rDescriptor);
    }

    // -----------------------------------------------------------------------------

    CMaterialPtr GetDefaultMaterialByHash()
    {
        return CGfxMaterialManager::GetInstance().GetDefaultMaterialByHash();
    }

    // -----------------------------------------------------------------------------

    CMaterialPtr GetMaterialByHash(unsigned int _Hash)
    {
        return CGfxMaterialManager::GetInstance().GetMaterialByHash(_Hash);
    }
} // namespace MaterialManager
} // namespace Gfx
