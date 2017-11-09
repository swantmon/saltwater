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
#include "graphic/gfx_sampler_manager.h"
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
    };

    const char* g_pShaderNamesPS[] =
    {
        "PSShaderMaterialDisney",
    };

    const char* g_pForwardShaderFilenamePS[] =
    {
        "fs_material_forward.glsl",
    };

    const char* g_pForwardShaderNamesPS[] =
    {
        "PSShaderMaterialDisneyForward",
    };

    // -----------------------------------------------------------------------------
    // Define shader defines
    // -----------------------------------------------------------------------------
    const char* g_pShaderAttributeDefines[] =
    {
        "#define USE_TEX_DIFFUSE\n"  ,
        "#define USE_TEX_NORMAL\n"   ,
        "#define USE_TEX_ROUGHNESS\n",
        "#define USE_TEX_METALLIC\n" ,
        "#define USE_TEX_AO\n"       ,
        "#define USE_TEX_BUMP\n"     ,
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

        typedef Base::CManagedPool<CInternMaterial, 32, 0> CMaterials;

        typedef std::unordered_map<unsigned int, CMaterialPtr> CMaterialByHashs;

    private:

        CMaterials   m_Materials;
        CMaterialPtr m_DefaultMaterialPtr;

        CMaterialByHashs m_MaterialByHash;

    private:

        void OnDirtyMaterial(Dt::CMaterial* _Material);

        CMaterialPtr InternCreateMaterial(const SMaterialDescriptor& _rDescriptor);

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
        MaterialDescriptor.m_Displacement    = 0.0f;
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
        // Clear hashes
        // -----------------------------------------------------------------------------
        m_MaterialByHash.clear();

        // -----------------------------------------------------------------------------
        // Clear materials
        // -----------------------------------------------------------------------------
        m_Materials.Clear();
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
            return m_MaterialByHash.at(Hash);
        }

        // -----------------------------------------------------------------------------
        // Material
        // -----------------------------------------------------------------------------
        CMaterialPtr MaterialPtr = InternCreateMaterial(_rDescriptor);

        CInternMaterial* pInternMaterial = static_cast<CInternMaterial*>(MaterialPtr.GetPtr());

        if (pInternMaterial == nullptr)
        {
            return m_DefaultMaterialPtr;
        }

        if (Hash != 0)
        {
            pInternMaterial->m_Hash = Hash;

            m_MaterialByHash[Hash] = pInternMaterial;
        }

        return MaterialPtr;
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

        if ((DirtyFlags & Dt::CMaterial::DirtyCreate) != 0)
        {
            if (m_MaterialByHash.find(Hash) == m_MaterialByHash.end())
            {
                SMaterialDescriptor MaterialDescriptor;

                // TODO by tschwandt
                // Do not use filename of texture; use texture instead
                MaterialDescriptor.m_pMaterialName   = _Material->GetMaterialname().length() > 0 ? _Material->GetMaterialname().c_str() : 0;
                MaterialDescriptor.m_pFileName       = _Material->GetFileName().length()     > 0 ? _Material->GetFileName().c_str()     : 0;

                MaterialDescriptor.m_pColorMap       = _Material->GetColorTexture()            != 0 ? _Material->GetColorTexture()->GetFileName().c_str()            : 0;
                MaterialDescriptor.m_pNormalMap      = _Material->GetNormalTexture()           != 0 ? _Material->GetNormalTexture()->GetFileName().c_str()           : 0;
                MaterialDescriptor.m_pRoughnessMap   = _Material->GetRoughnessTexture()        != 0 ? _Material->GetRoughnessTexture()->GetFileName().c_str()        : 0;
                MaterialDescriptor.m_pMetalMaskMap   = _Material->GetMetalTexture()            != 0 ? _Material->GetMetalTexture()->GetFileName().c_str()            : 0;
                MaterialDescriptor.m_pAOMap          = _Material->GetAmbientOcclusionTexture() != 0 ? _Material->GetAmbientOcclusionTexture()->GetFileName().c_str() : 0;
                MaterialDescriptor.m_pBumpMap        = _Material->GetBumpTexture()             != 0 ? _Material->GetBumpTexture()->GetFileName().c_str()             : 0;

                MaterialDescriptor.m_Roughness       = _Material->GetRoughness();
                MaterialDescriptor.m_Reflectance     = _Material->GetReflectance();
                MaterialDescriptor.m_MetalMask       = _Material->GetMetalness();
                MaterialDescriptor.m_Displacement    = _Material->GetDisplacement();
                MaterialDescriptor.m_AlbedoColor     = _Material->GetColor();
                MaterialDescriptor.m_TilingOffset    = _Material->GetTilingOffset();

                CMaterialPtr MaterialPtr = InternCreateMaterial(MaterialDescriptor);

                CInternMaterial* pInternMaterial = static_cast<CInternMaterial*>(MaterialPtr.GetPtr());

                if (pInternMaterial != nullptr)
                {
                    pInternMaterial->m_Hash = Hash;

                    m_MaterialByHash[Hash] = pInternMaterial;
                }
            }
            else
            {
                BASE_CONSOLE_STREAMDEBUG("A material has already been created. No new one will be created.")
            }
        }

        if ((DirtyFlags & Dt::CMaterial::DirtyData) != 0)
        {
            // -----------------------------------------------------------------------------
            // Get data material
            // -----------------------------------------------------------------------------
            Dt::CMaterial& rDataMaterial    = *_Material;

            CMaterialPtr MaterialPtr = m_MaterialByHash.at(Hash);

            CInternMaterial* pGraphicMaterial = static_cast<CInternMaterial*>(MaterialPtr.GetPtr());

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
            rMaterial.m_HasBump  = rDataMaterial.GetBumpTexture() != 0;

            // -----------------------------------------------------------------------------
            // Shader
            // -----------------------------------------------------------------------------
            SetShaderOfMaterial(rMaterial);

            // -----------------------------------------------------------------------------
            // Create and setup shader, texture and sampler (setup material)
            // -----------------------------------------------------------------------------
            CTextureBasePtr TexturePtrs[CMaterial::SMaterialKey::s_NumberOfTextures];

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
        }

        if ((DirtyFlags & Dt::CMaterial::DirtyData) != 0)
        {
            // -----------------------------------------------------------------------------
            // Get data material
            // -----------------------------------------------------------------------------
            Dt::CMaterial&   rDataMaterial    = *_Material;

            CMaterialPtr MaterialPtr = m_MaterialByHash.at(Hash);

            CInternMaterial* pGraphicMaterial = static_cast<CInternMaterial*>(MaterialPtr.GetPtr());

            if (pGraphicMaterial == nullptr)
            {
                BASE_CONSOLE_STREAMWARNING("Update of data material failed because it is not created!");

                return;
            }

            CInternMaterial& rMaterial = *pGraphicMaterial;

            // -----------------------------------------------------------------------------
            // Set definitions
            // -----------------------------------------------------------------------------
            rMaterial.m_HasAlpha = false;

            // -----------------------------------------------------------------------------
            // Set attributes
            // -----------------------------------------------------------------------------
            rMaterial.m_MaterialAttributes.m_Color        = rDataMaterial.GetColor();
            rMaterial.m_MaterialAttributes.m_Roughness    = rDataMaterial.GetRoughness();
            rMaterial.m_MaterialAttributes.m_Reflectance  = rDataMaterial.GetReflectance();
            rMaterial.m_MaterialAttributes.m_MetalMask    = rDataMaterial.GetMetalness();
            rMaterial.m_MaterialAttributes.m_Displacement = rDataMaterial.GetDisplacement();
            rMaterial.m_MaterialAttributes.m_TilingOffset = rDataMaterial.GetTilingOffset();
        }
    }

    // -----------------------------------------------------------------------------

    CMaterialPtr CGfxMaterialManager::InternCreateMaterial(const SMaterialDescriptor& _rDescriptor)
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
        float                 Displacement;
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
        Displacement    = _rDescriptor.m_Displacement;
        AlbedoColor     = _rDescriptor.m_AlbedoColor;
        TilingOffset    = _rDescriptor.m_TilingOffset;

        if (_rDescriptor.m_pFileName != nullptr && strlen(_rDescriptor.m_pFileName) > 0)
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
        CMaterialPtr MaterialPtr = static_cast<CMaterialPtr>(m_Materials.Allocate());

        CInternMaterial* pInternMaterial = static_cast<CInternMaterial*>(MaterialPtr.GetPtr());
        
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
        rMaterial.m_MaterialAttributes.m_Displacement = Displacement;
        rMaterial.m_MaterialAttributes.m_TilingOffset = TilingOffset;

        // -----------------------------------------------------------------------------
        // Shader
        // -----------------------------------------------------------------------------
        SetShaderOfMaterial(rMaterial);

        // -----------------------------------------------------------------------------
        // Create and setup shader, texture and sampler (setup material)
        // -----------------------------------------------------------------------------
        CTextureBasePtr    TexturePtrs[CMaterial::SMaterialKey::s_NumberOfTextures];
        CSamplerPtr        SamplerPtrs[CMaterial::SMaterialKey::s_NumberOfTextures];
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

        // -----------------------------------------------------------------------------
        // Textures
        // -----------------------------------------------------------------------------
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

        // -----------------------------------------------------------------------------
        // Sampler
        // -----------------------------------------------------------------------------
        SamplerPtrs[0] = SamplerManager::GetSampler(CSampler::MinMagMipLinearWrap);
        SamplerPtrs[1] = SamplerManager::GetSampler(CSampler::MinMagMipLinearWrap);
        SamplerPtrs[2] = SamplerManager::GetSampler(CSampler::MinMagMipLinearWrap);
        SamplerPtrs[3] = SamplerManager::GetSampler(CSampler::MinMagMipLinearWrap);
        SamplerPtrs[4] = SamplerManager::GetSampler(CSampler::MinMagMipLinearWrap);
        SamplerPtrs[5] = SamplerManager::GetSampler(CSampler::MinMagMipLinearWrap);

        rMaterial.m_SamplerSetPtrs[CShader::Pixel] = SamplerManager::CreateSamplerSet(SamplerPtrs, CMaterial::SMaterialKey::s_NumberOfTextures);

        return MaterialPtr;
    }

    // -----------------------------------------------------------------------------

    void CGfxMaterialManager::SetShaderOfMaterial(CInternMaterial& _rMaterial) const
    {
        std::string Defines = "";
          
        if (_rMaterial.m_MaterialKey.m_HasDiffuseTex)   Defines += g_pShaderAttributeDefines[0];
        if (_rMaterial.m_MaterialKey.m_HasNormalTex)    Defines += g_pShaderAttributeDefines[1];
        if (_rMaterial.m_MaterialKey.m_HasRoughnessTex) Defines += g_pShaderAttributeDefines[2];
        if (_rMaterial.m_MaterialKey.m_HasMetallicTex)  Defines += g_pShaderAttributeDefines[3];
        if (_rMaterial.m_MaterialKey.m_HasAOTex)        Defines += g_pShaderAttributeDefines[4];
        if (_rMaterial.m_MaterialKey.m_HasBumpTex)      Defines += g_pShaderAttributeDefines[5];

        const char* pDefineString = 0;

        if (Defines.length() > 0)
        {
            pDefineString = Defines.c_str();
        }

        // -----------------------------------------------------------------------------
        // Bump Mapping
        // -----------------------------------------------------------------------------
        if (_rMaterial.m_HasBump)
        {
            _rMaterial.m_ShaderPtrs[CShader::Hull] = Gfx::ShaderManager::CompileHS(g_pShaderFilenameHS[0], g_pShaderNamesHS[0], pDefineString);

            _rMaterial.m_ShaderPtrs[CShader::Domain] = Gfx::ShaderManager::CompileDS(g_pShaderFilenameDS[0], g_pShaderNamesDS[0], pDefineString);
        }

        // -----------------------------------------------------------------------------
        // Disney
        // -----------------------------------------------------------------------------
        _rMaterial.m_ShaderPtrs[CShader::Pixel] = Gfx::ShaderManager::CompilePS(g_pShaderFilenamePS[0], g_pShaderNamesPS[0], pDefineString);

        _rMaterial.m_ForwardShaderPSPtr = Gfx::ShaderManager::CompilePS(g_pForwardShaderFilenamePS[0], g_pForwardShaderNamesPS[0], pDefineString);
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
