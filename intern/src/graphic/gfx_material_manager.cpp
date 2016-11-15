//
//  gfx_material_manager.cpp
//  graphic
//
//  Created by Tobias Schwandt on 22/04/15.
//  Copyright (c) 2015 TU Ilmenau. All rights reserved.
//

#include "graphic/gfx_precompiled.h"

#include "base/base_console.h"
#include "base/base_crc.h"
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
                unsigned int m_pShaderNamesHS;
                unsigned int m_pShaderNamesDS;
                unsigned int m_pShaderNamesGS;
                unsigned int m_pShaderNamesPS;
            };
            unsigned int m_ShaderType[5];
        };
    };
#pragma warning(default:4201)

    static const unsigned int s_NumberOfShaderMaterialCombinations = 9;

    const SShaderShaderLink g_ShaderShaderLinks[s_NumberOfShaderMaterialCombinations] =
    {
        { s_NoShader, s_NoShader, s_NoShader, 0 },
        { s_NoShader, s_NoShader, s_NoShader, 1 },
        { s_NoShader, s_NoShader, s_NoShader, 2 },
        { s_NoShader, s_NoShader, s_NoShader, 3 },
        { s_NoShader, s_NoShader, s_NoShader, 4 },
        { s_NoShader, s_NoShader, s_NoShader, 5 },
        { s_NoShader, s_NoShader, s_NoShader, 6 },
        { 0         , 0         , s_NoShader, 2 },
        { 0         , 0         , s_NoShader, 3 },
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
        // 05. Attribute: HasMetallicTex
        // 06. Attribute: HasAOTex
        // 07. Attribute: HasBumpTex
        // -----------------------------------------------------------------------------

        // 01  , 02   , 03   , 04   , 05,    06
        { false, false, false, false, false, false },
        { false, true , false, false, false, false },
        { true , false, false, false, false, false },
        { true , true , false, false, false, false },
        { true , true , true , false, false, false },
        { true , true , true , true , false, false },
        { true , true , true , true , true , false },
        { true , false, false, false, false, true  },
        { true , true , false, false, false, true  },
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

        CMaterialPtr GetDefaultMaterial();

        CMaterialPtr GetMaterialByHash(unsigned int _Hash);

    private:

        class CInternMaterial : public CMaterial
        {
        private:

            friend class CGfxMaterialManager;
        };

    private:

        typedef Base::CManagedPool<CInternMaterial, 1024, 1> CMaterials;

        typedef std::unordered_map<unsigned int, CInternMaterial*> CMaterialByHashs;

    private:

        CMaterials   m_Materials;
        CMaterialPtr m_DefaultMaterialPtr;

        CMaterialByHashs m_MaterialByHash;

    private:

        void OnDirtyMaterial(Dt::CMaterial* _Material);

        CInternMaterial* InternCreateMaterial(const SMaterialDescriptor& _rDescriptor);

        void SetShaderOfMaterial(CInternMaterial& _rMaterial) const;
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
        SMaterialDescriptor MaterialDescriptor;

        MaterialDescriptor.m_pMaterialName   = "STATIC DEFAULT MATERIAL";
        MaterialDescriptor.m_pColorMap       = 0;
        MaterialDescriptor.m_pNormalMap      = 0;
        MaterialDescriptor.m_pRoughnessMap   = 0;
        MaterialDescriptor.m_pMetalMaskMap   = 0;
        MaterialDescriptor.m_pAOMap          = 0;
        MaterialDescriptor.m_pBumpMap        = 0;
        MaterialDescriptor.m_Roughness       = 1.0f;
        MaterialDescriptor.m_Reflectance     = 0.0f;
        MaterialDescriptor.m_MetalMask       = 0.0f;
        MaterialDescriptor.m_AlbedoColor     = Base::Float3(1.0f);
        MaterialDescriptor.m_TilingOffset    = Base::Float4(0.0f);
        MaterialDescriptor.m_pFileName       = 0;

        m_DefaultMaterialPtr = CreateMaterial(MaterialDescriptor);

        // -----------------------------------------------------------------------------
        // Set dirty handler of data textures
        // -----------------------------------------------------------------------------
        Dt::MaterialManager::RegisterDirtyMaterialHandler(DATA_DIRTY_MATERIAL_METHOD(&CGfxMaterialManager::OnDirtyMaterial));
    }

    // -----------------------------------------------------------------------------

    void CGfxMaterialManager::OnExit()
    {
        m_DefaultMaterialPtr = 0;

        // -----------------------------------------------------------------------------
        // Clear materials
        // -----------------------------------------------------------------------------
        m_Materials.Clear();

        // -----------------------------------------------------------------------------
        // Clear hashes
        // -----------------------------------------------------------------------------
        m_MaterialByHash.clear();
    }

    // -----------------------------------------------------------------------------

    CMaterialPtr CGfxMaterialManager::CreateMaterial(const SMaterialDescriptor& _rDescriptor)
    {
        int          NumberOfBytes;
        unsigned int Hash;

        Hash = 0;

        // -----------------------------------------------------------------------------
        // Create hash value over filename
        // -----------------------------------------------------------------------------
        if (_rDescriptor.m_pFileName != nullptr && strlen(_rDescriptor.m_pFileName))
        {
            NumberOfBytes     = static_cast<unsigned int>(strlen(_rDescriptor.m_pFileName) * sizeof(char));
            const void* pData = static_cast<const void*>(_rDescriptor.m_pFileName);

            Hash = Base::CRC32(pData, NumberOfBytes);
        }
        else if (_rDescriptor.m_pMaterialName != nullptr && strlen(_rDescriptor.m_pMaterialName))
        {
            NumberOfBytes     = static_cast<unsigned int>(strlen(_rDescriptor.m_pMaterialName) * sizeof(char));
            const void* pData = static_cast<const void*>(_rDescriptor.m_pMaterialName);

            Hash = Base::CRC32(pData, NumberOfBytes);
        }

        if (m_MaterialByHash.find(Hash) != m_MaterialByHash.end())
        {
            return CMaterialPtr(m_MaterialByHash.at(Hash));
        }

        // -----------------------------------------------------------------------------
        // Material
        // -----------------------------------------------------------------------------
        CInternMaterial* pInternMaterial = InternCreateMaterial(_rDescriptor);

        if (pInternMaterial == nullptr)
        {
            return m_DefaultMaterialPtr;
        }

        if (Hash != 0)
        {
            pInternMaterial->m_Hash = Hash;

            m_MaterialByHash[Hash] = pInternMaterial;
        }

        return CMaterialPtr(pInternMaterial);
    }

    // -----------------------------------------------------------------------------

    CMaterialPtr CGfxMaterialManager::GetDefaultMaterial()
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

        assert(Hash != 0);

        if ((DirtyFlags & Dt::CTextureBase::DirtyCreate) != 0)
        {
            SMaterialDescriptor MaterialDescriptor;

            // TODO by tschwandt
            // Do not use filename of texture; use texture instead
            MaterialDescriptor.m_pMaterialName   = _Material->GetMaterialname();
            MaterialDescriptor.m_pColorMap       = _Material->GetColorTexture()            != 0 ? _Material->GetColorTexture()->GetFileName()            : 0;
            MaterialDescriptor.m_pNormalMap      = _Material->GetNormalTexture()           != 0 ? _Material->GetNormalTexture()->GetFileName()           : 0;
            MaterialDescriptor.m_pRoughnessMap   = _Material->GetRoughnessTexture()        != 0 ? _Material->GetRoughnessTexture()->GetFileName()        : 0;
            MaterialDescriptor.m_pMetalMaskMap   = _Material->GetMetalTexture()            != 0 ? _Material->GetMetalTexture()->GetFileName()            : 0;
            MaterialDescriptor.m_pAOMap          = _Material->GetAmbientOcclusionTexture() != 0 ? _Material->GetAmbientOcclusionTexture()->GetFileName() : 0;
            MaterialDescriptor.m_pBumpMap        = _Material->GetBumpTexture()             != 0 ? _Material->GetBumpTexture()->GetFileName()             : 0;
            MaterialDescriptor.m_Roughness       = _Material->GetRoughness();
            MaterialDescriptor.m_Reflectance     = _Material->GetReflectance();
            MaterialDescriptor.m_MetalMask       = _Material->GetMetalness();
            MaterialDescriptor.m_AlbedoColor     = _Material->GetColor();
            MaterialDescriptor.m_TilingOffset    = _Material->GetTilingOffset();
            MaterialDescriptor.m_pFileName       = _Material->GetFileName();

            CInternMaterial* pInternMaterial = InternCreateMaterial(MaterialDescriptor);

            if (pInternMaterial != nullptr)
            {
                pInternMaterial->m_Hash = Hash;

                m_MaterialByHash[Hash] = pInternMaterial;
            }
        }

        if ((DirtyFlags & Dt::CTextureBase::DirtyData) != 0)
        {
            // -----------------------------------------------------------------------------
            // Get data material
            // -----------------------------------------------------------------------------
            Dt::CMaterial&   rDataMaterial    = *_Material;
            CInternMaterial* pGraphicMaterial = m_MaterialByHash.at(Hash);

            if (pGraphicMaterial == nullptr)
            {
                BASE_CONSOLE_STREAMWARNING("Update of data material failed because it is not created!");

                return;
            }

            CInternMaterial& rMaterial = *pGraphicMaterial;

            // -----------------------------------------------------------------------------
            // Key estimation
            // -----------------------------------------------------------------------------
            rMaterial.m_MaterialKey.m_HasDiffuseTex     = rDataMaterial.GetColorTexture()            != 0;
            rMaterial.m_MaterialKey.m_HasNormalTex      = rDataMaterial.GetNormalTexture()           != 0;
            rMaterial.m_MaterialKey.m_HasRoughnessTex   = rDataMaterial.GetRoughnessTexture()        != 0;
            rMaterial.m_MaterialKey.m_HasMetallicTex    = rDataMaterial.GetMetalTexture()            != 0;
            rMaterial.m_MaterialKey.m_HasAOTex          = rDataMaterial.GetAmbientOcclusionTexture() != 0;
            rMaterial.m_MaterialKey.m_HasBumpTex        = rDataMaterial.GetBumpTexture()             != 0;

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
            // Shader
            // -----------------------------------------------------------------------------
            SetShaderOfMaterial(rMaterial);

            // -----------------------------------------------------------------------------
            // Create and setup shader, texture and sampler (setup material)
            // -----------------------------------------------------------------------------
            CTextureBasePtr    TexturePtrs[CMaterial::SMaterialKey::s_NumberOfTextures];

            TexturePtrs[0] = 0;
            TexturePtrs[1] = 0;
            TexturePtrs[2] = 0;
            TexturePtrs[3] = 0;
            TexturePtrs[4] = 0;
            TexturePtrs[5] = 0;

            if (rMaterial.m_MaterialKey.m_HasDiffuseTex)
            {
                unsigned int Hash = rDataMaterial.GetColorTexture()->GetHash();

                TexturePtrs[0] = TextureManager::GetTexture2DByHash(Hash);
            }

            if (rMaterial.m_MaterialKey.m_HasNormalTex)
            {
                unsigned int Hash = rDataMaterial.GetNormalTexture()->GetHash();

                TexturePtrs[1] = TextureManager::GetTexture2DByHash(Hash);
            }

            if (rMaterial.m_MaterialKey.m_HasRoughnessTex)
            {
                unsigned int Hash = rDataMaterial.GetRoughnessTexture()->GetHash();

                TexturePtrs[2] = TextureManager::GetTexture2DByHash(Hash);
            }

            if (rMaterial.m_MaterialKey.m_HasMetallicTex)
            {
                unsigned int Hash = rDataMaterial.GetMetalTexture()->GetHash();

                TexturePtrs[3] = TextureManager::GetTexture2DByHash(Hash);
            }

            if (rMaterial.m_MaterialKey.m_HasAOTex)
            {
                unsigned int Hash = rDataMaterial.GetAmbientOcclusionTexture()->GetHash();

                TexturePtrs[4] = TextureManager::GetTexture2DByHash(Hash);
            }

            if (rMaterial.m_MaterialKey.m_HasBumpTex)
            {
                unsigned int Hash = rDataMaterial.GetBumpTexture()->GetHash();

                TexturePtrs[5] = TextureManager::GetTexture2DByHash(Hash);
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
        const char*           pMaterialName;
        const char*           pColorMap;
        const char*           pNormalMap;
        const char*           pRoughnessMap;
        const char*           pMetalMaskMap;
        const char*           pAOMap;
        const char*           pBumpMap;
        float                 Roughness;
        float                 Reflectance;
        float                 MetalMask;
        Base::Float3          AlbedoColor;
        Base::Float4          TilingOffset;
        tinyxml2::XMLDocument MaterialFile;

        pMaterialName   = _rDescriptor.m_pMaterialName;
        pColorMap       = _rDescriptor.m_pColorMap;
        pNormalMap      = _rDescriptor.m_pNormalMap;
        pRoughnessMap   = _rDescriptor.m_pRoughnessMap;
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
        rMaterial.m_MaterialKey.m_HasDiffuseTex     = pColorMap       != 0;
        rMaterial.m_MaterialKey.m_HasNormalTex      = pNormalMap      != 0;
        rMaterial.m_MaterialKey.m_HasRoughnessTex   = pRoughnessMap   != 0;
        rMaterial.m_MaterialKey.m_HasMetallicTex    = pMetalMaskMap   != 0;
        rMaterial.m_MaterialKey.m_HasAOTex          = pAOMap          != 0;
        rMaterial.m_MaterialKey.m_HasBumpTex        = pBumpMap        != 0;

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
        // Shader
        // -----------------------------------------------------------------------------
        SetShaderOfMaterial(rMaterial);

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

        if (rMaterial.m_MaterialKey.m_HasMetallicTex)
        {
            TextureDescriptor.m_pFileName = pMetalMaskMap;

            TexturePtrs[3] = TextureManager::CreateTexture2D(TextureDescriptor);
        }

        if (rMaterial.m_MaterialKey.m_HasAOTex)
        {
            TextureDescriptor.m_pFileName = pAOMap;

            TexturePtrs[4] = TextureManager::CreateTexture2D(TextureDescriptor);
        }

        if (rMaterial.m_MaterialKey.m_HasBumpTex)
        {
            TextureDescriptor.m_NumberOfPixelsW = 1;
            TextureDescriptor.m_Format          = CTextureBase::R8_UBYTE;
            TextureDescriptor.m_pFileName       = pBumpMap;

            TexturePtrs[5] = TextureManager::CreateTexture2D(TextureDescriptor);
        }

        rMaterial.m_TextureSetPtrs[CShader::Pixel] = TextureManager::CreateTextureSet(TexturePtrs, CMaterial::SMaterialKey::s_NumberOfTextures);

        if (_rDescriptor.m_pFileName != 0)
        {
            MaterialFile.Clear();
        }

        return pInternMaterial;
    }

    // -----------------------------------------------------------------------------

    void CGfxMaterialManager::SetShaderOfMaterial(CInternMaterial& _rMaterial) const
    {
        unsigned int ShaderLinkIndex = 0;

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

    CMaterialPtr GetDefaultMaterial()
    {
        return CGfxMaterialManager::GetInstance().GetDefaultMaterial();
    }

    // -----------------------------------------------------------------------------

    CMaterialPtr GetMaterialByHash(unsigned int _Hash)
    {
        return CGfxMaterialManager::GetInstance().GetMaterialByHash(_Hash);
    }
} // namespace MaterialManager
} // namespace Gfx
