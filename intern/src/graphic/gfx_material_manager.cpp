
#include "graphic/gfx_precompiled.h"

#include "base/base_component_manager.h"
#include "base/base_console.h"
#include "base/base_crc.h"
#include "base/base_exception.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "core/core_asset_importer.h"
#include "core/core_asset_manager.h"

#include "data/data_component.h"
#include "data/data_entity.h"
#include "data/data_material_helper.h"

#include "graphic/gfx_material.h"
#include "graphic/gfx_material_manager.h"
#include "graphic/gfx_texture_manager.h"
#include "graphic/gfx_sampler_manager.h"
#include "graphic/gfx_shader.h"
#include "graphic/gfx_shader_manager.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"

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
    std::string g_PathToDataModels = "/graphic/models/";
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

        CMaterialPtr CreateMaterialFromFile(const Base::Char* _pFilename, int _MaterialIndex = 0);

        const CMaterialPtr GetDefaultMaterial();

    private:

        class CInternMaterial : public CMaterial
        {
        private:

            friend class CGfxMaterialManager;
        };

    private:

        typedef Base::CManagedPool<CInternMaterial, 32, 1> CMaterials;

    private:

        CMaterials   m_Materials;
        CMaterialPtr m_DefaultMaterialPtr;

    private:

        void OnDirtyComponent(Base::IComponent* _pComponent);

        void FillMaterialFromXML(CInternMaterial* _pMaterial, const std::string& _rFilename);

        void FillMaterialFromAssimpFile(CInternMaterial* _pMaterial, const std::string& _rFilename, int _MaterialIndex);

        void FillMaterialFromAssimpMaterial(CInternMaterial* _pMaterial, const aiMaterial* _pAssimpMaterial);

        void FillMaterialFromData(CInternMaterial* _pMaterial, const SMaterialDescriptor& _rDescription);

        void SetShaderOfMaterial(CInternMaterial& _rMaterial) const;
    };
} // namespace

namespace
{
    CGfxMaterialManager::CGfxMaterialManager()
        : m_DefaultMaterialPtr(0)
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

        MaterialDescriptor.m_pMaterialName = "STATIC CONST DEFAULT GFX MATERIAL"; 
        MaterialDescriptor.m_pColorMap     = 0; 
        MaterialDescriptor.m_pNormalMap    = 0; 
        MaterialDescriptor.m_pRoughnessMap = 0; 
        MaterialDescriptor.m_pMetalMaskMap = 0; 
        MaterialDescriptor.m_pAOMap        = 0; 
        MaterialDescriptor.m_pBumpMap      = 0; 
        MaterialDescriptor.m_Roughness     = 1.0f; 
        MaterialDescriptor.m_Reflectance   = 0.0f; 
        MaterialDescriptor.m_MetalMask     = 0.0f; 
        MaterialDescriptor.m_AlbedoColor   = glm::vec3(0.8f); 
        MaterialDescriptor.m_TilingOffset  = glm::vec4(1.0f, 1.0f, 0.0f, 0.0f);

        m_DefaultMaterialPtr = CreateMaterial(MaterialDescriptor);

        // -----------------------------------------------------------------------------
        // Set dirty handler of data textures
        // -----------------------------------------------------------------------------
        Dt::CComponentManager::GetInstance().RegisterDirtyComponentHandler(BASE_DIRTY_COMPONENT_METHOD(&CGfxMaterialManager::OnDirtyComponent));
    }

    // -----------------------------------------------------------------------------

    void CGfxMaterialManager::OnExit()
    {
        m_DefaultMaterialPtr = 0;

        m_Materials.Clear();
    }

    // -----------------------------------------------------------------------------

    CMaterialPtr CGfxMaterialManager::CreateMaterial(const SMaterialDescriptor& _rDescriptor)
    {
        auto pMaterial = m_Materials.Allocate();

        FillMaterialFromData(pMaterial, _rDescriptor);

        return CMaterialPtr(pMaterial);
    }

    // -----------------------------------------------------------------------------

    CMaterialPtr CGfxMaterialManager::CreateMaterialFromFile(const Base::Char* _pFilename, int _MaterialIndex)
    {
        if (_pFilename != 0)
        {
            auto pMaterial = m_Materials.Allocate();

            std::string MaterialFileName = _pFilename;

            if (MaterialFileName.find(".mat") != std::string::npos)
            {
                FillMaterialFromXML(pMaterial, MaterialFileName);
            }
            else
            {
                FillMaterialFromAssimpFile(pMaterial, MaterialFileName, _MaterialIndex);
            }

            return CMaterialPtr(pMaterial);
        }

        return 0;
    }

    // -----------------------------------------------------------------------------

    const CMaterialPtr CGfxMaterialManager::GetDefaultMaterial()
    {
        return m_DefaultMaterialPtr;
    }

    // -----------------------------------------------------------------------------

    void CGfxMaterialManager::OnDirtyComponent(Base::IComponent* _pComponent)
    {
        if (_pComponent->GetTypeID() != Base::CTypeInfo::GetTypeID<Dt::CMaterialComponent>()) return;

        Dt::CMaterialComponent* pMaterialComponent = static_cast<Dt::CMaterialComponent*>(_pComponent);

        // -----------------------------------------------------------------------------
        // Dirty check
        // -----------------------------------------------------------------------------
        unsigned int DirtyFlags;

        DirtyFlags = pMaterialComponent->GetDirtyFlags();

        // -----------------------------------------------------------------------------
        // Material
        // -----------------------------------------------------------------------------
        Base::ID ID = pMaterialComponent->GetID();

        assert(ID != 0);

        CInternMaterial* pInternMaterial = 0;

        if ((DirtyFlags & Dt::CMaterialComponent::DirtyCreate) != 0)
        {
            pInternMaterial = m_Materials.Allocate();

            pMaterialComponent->SetFacet(Dt::CMaterialComponent::Graphic, pInternMaterial);
        }
        else if ((DirtyFlags & Dt::CMaterialComponent::DirtyInfo) != 0)
        {
            pInternMaterial = static_cast<CInternMaterial*>(pMaterialComponent->GetFacet(Dt::CMaterialComponent::Graphic));
        }

        if (pInternMaterial)
        {
            if (pMaterialComponent->GetFileName().length() > 0)
            {
                std::string MaterialExaminer = pMaterialComponent->GetFileName();

                if (MaterialExaminer.find(".mat") != std::string::npos)
                {
                    FillMaterialFromXML(pInternMaterial, pMaterialComponent->GetFileName());
                }
                else
                {
                    FillMaterialFromAssimpFile(pInternMaterial, pMaterialComponent->GetFileName(), 0);
                }
            }
            else
            {
                SMaterialDescriptor MaterialDescriptor;

                MaterialDescriptor.m_pMaterialName = pMaterialComponent->GetMaterialname().length() > 0 ? pMaterialComponent->GetMaterialname().c_str() : 0;

                MaterialDescriptor.m_Roughness    = pMaterialComponent->GetRoughness();
                MaterialDescriptor.m_Reflectance  = pMaterialComponent->GetReflectance();
                MaterialDescriptor.m_MetalMask    = pMaterialComponent->GetMetalness();
                MaterialDescriptor.m_Displacement = pMaterialComponent->GetDisplacement();
                MaterialDescriptor.m_AlbedoColor  = pMaterialComponent->GetColor();
                MaterialDescriptor.m_TilingOffset = pMaterialComponent->GetTilingOffset();

                MaterialDescriptor.m_pColorMap     = pMaterialComponent->GetColorTexture().length() > 0 ? pMaterialComponent->GetColorTexture().c_str() : 0;
                MaterialDescriptor.m_pNormalMap    = pMaterialComponent->GetNormalTexture().length() > 0 ? pMaterialComponent->GetNormalTexture().c_str() : 0;
                MaterialDescriptor.m_pRoughnessMap = pMaterialComponent->GetRoughnessTexture().length() > 0 ? pMaterialComponent->GetRoughnessTexture().c_str() : 0;
                MaterialDescriptor.m_pMetalMaskMap = pMaterialComponent->GetMetalTexture().length() > 0 ? pMaterialComponent->GetMetalTexture().c_str() : 0;
                MaterialDescriptor.m_pAOMap        = pMaterialComponent->GetAmbientOcclusionTexture().length() > 0 ? pMaterialComponent->GetAmbientOcclusionTexture().c_str() : 0;
                MaterialDescriptor.m_pBumpMap      = pMaterialComponent->GetBumpTexture().length() > 0 ? pMaterialComponent->GetBumpTexture().c_str() : 0;

                FillMaterialFromData(pInternMaterial, MaterialDescriptor);
            }
        }
    }

    // -----------------------------------------------------------------------------

    void CGfxMaterialManager::FillMaterialFromXML(CInternMaterial* _pMaterial, const std::string& _rFilename)
    {
        // -----------------------------------------------------------------------------
        // Build path to texture in file system
        // -----------------------------------------------------------------------------
        std::string PathToMaterial = Core::AssetManager::GetPathToAssets() + "/" + _rFilename;

        // -----------------------------------------------------------------------------
        // Load material file
        // -----------------------------------------------------------------------------
        auto Importer = Core::AssetImporter::AllocateTinyXMLImporter(PathToMaterial);

        tinyxml2::XMLDocument* pMaterialFile = static_cast<tinyxml2::XMLDocument*>(Core::AssetImporter::GetNativeAccessFromImporter(Importer));

        tinyxml2::XMLElement* pElementDefinition  = pMaterialFile->FirstChildElement("MaterialDefinition");
        tinyxml2::XMLElement* pElementColor       = pElementDefinition->FirstChildElement("Color");
        tinyxml2::XMLElement* pElementNormal      = pElementDefinition->FirstChildElement("Normal");
        tinyxml2::XMLElement* pElementRoughness   = pElementDefinition->FirstChildElement("Roughness");
        tinyxml2::XMLElement* pElementReflectance = pElementDefinition->FirstChildElement("Reflectance");
        tinyxml2::XMLElement* pElementMetallic    = pElementDefinition->FirstChildElement("Metallic");
        tinyxml2::XMLElement* pElementAO          = pElementDefinition->FirstChildElement("AO");
        tinyxml2::XMLElement* pElementBump        = pElementDefinition->FirstChildElement("Bump");
        tinyxml2::XMLElement* pElementTiling      = pElementDefinition->FirstChildElement("Tiling");
        tinyxml2::XMLElement* pElementOffset      = pElementDefinition->FirstChildElement("Offset");

        // -----------------------------------------------------------------------------
        // Descriptor
        // -----------------------------------------------------------------------------
        SMaterialDescriptor MaterialDescriptor;

        MaterialDescriptor.m_pMaterialName = 0; 
        MaterialDescriptor.m_pColorMap     = 0; 
        MaterialDescriptor.m_pNormalMap    = 0; 
        MaterialDescriptor.m_pRoughnessMap = 0; 
        MaterialDescriptor.m_pMetalMaskMap = 0; 
        MaterialDescriptor.m_pAOMap        = 0; 
        MaterialDescriptor.m_pBumpMap      = 0; 
        MaterialDescriptor.m_Roughness     = 1.0f; 
        MaterialDescriptor.m_Reflectance   = 0.0f; 
        MaterialDescriptor.m_MetalMask     = 0.0f; 
        MaterialDescriptor.m_AlbedoColor   = glm::vec3(1.0f); 
        MaterialDescriptor.m_TilingOffset  = glm::vec4(1.0f, 1.0f, 0.0f, 0.0f);

        // -----------------------------------------------------------------------------
        // Values
        // -----------------------------------------------------------------------------
        MaterialDescriptor.m_pMaterialName = pElementDefinition->Attribute("Name");

        if (pElementColor != 0)
        {
            float ColorR = pElementColor->FloatAttribute("R");
            float ColorG = pElementColor->FloatAttribute("G");
            float ColorB = pElementColor->FloatAttribute("B");

            MaterialDescriptor.m_AlbedoColor = glm::vec3(ColorR, ColorG, ColorB);
        }

        if (pElementRoughness) MaterialDescriptor.m_Roughness = pElementRoughness->FloatAttribute("V");
        if (pElementReflectance) MaterialDescriptor.m_Reflectance = pElementReflectance->FloatAttribute("V");
        if (pElementMetallic) MaterialDescriptor.m_MetalMask = pElementMetallic->FloatAttribute("V");
        if (pElementBump) MaterialDescriptor.m_Displacement = pElementBump->FloatAttribute("V");

        if (pElementTiling)
        {
            MaterialDescriptor.m_TilingOffset[0] = pElementTiling->FloatAttribute("X");
            MaterialDescriptor.m_TilingOffset[1] = pElementTiling->FloatAttribute("Y");
        }

        if (pElementOffset)
        {
            MaterialDescriptor.m_TilingOffset[2] = pElementOffset->FloatAttribute("X");
            MaterialDescriptor.m_TilingOffset[3] = pElementOffset->FloatAttribute("Y");
        }

        // -----------------------------------------------------------------------------
        // Textures
        // -----------------------------------------------------------------------------
        if (pElementColor != nullptr && pElementColor->Attribute("Map")) MaterialDescriptor.m_pColorMap = pElementColor->Attribute("Map");
        if (pElementNormal != nullptr && pElementNormal->Attribute("Map")) MaterialDescriptor.m_pNormalMap = pElementNormal->Attribute("Map");
        if (pElementRoughness != nullptr && pElementRoughness->Attribute("Map")) MaterialDescriptor.m_pRoughnessMap = pElementRoughness->Attribute("Map");
        if (pElementMetallic != nullptr && pElementMetallic->Attribute("Map")) MaterialDescriptor.m_pMetalMaskMap = pElementMetallic->Attribute("Map");
        if (pElementBump != nullptr && pElementBump->Attribute("Map")) MaterialDescriptor.m_pBumpMap = pElementBump->Attribute("Map");
        if (pElementAO != nullptr && pElementAO->Attribute("Map")) MaterialDescriptor.m_pAOMap = pElementAO->Attribute("Map");

        // -----------------------------------------------------------------------------
        // Release importer
        // -----------------------------------------------------------------------------
        Core::AssetImporter::ReleaseImporter(Importer);

        // -----------------------------------------------------------------------------
        // Fill data
        // -----------------------------------------------------------------------------
        FillMaterialFromData(_pMaterial, MaterialDescriptor);
    }

    // -----------------------------------------------------------------------------

    void CGfxMaterialManager::FillMaterialFromAssimpFile(CInternMaterial* _pMaterial, const std::string& _rFilename, int _MaterialIndex)
    {
        std::string PathToMaterial = Core::AssetManager::GetPathToAssets() + "/" + _rFilename;

        auto Importer = Core::AssetImporter::AllocateAssimpImporter(PathToMaterial, Core::AssetImporter::SGeneratorFlag::Nothing);

        const Assimp::Importer* pImporter = static_cast<const Assimp::Importer*>(Core::AssetImporter::GetNativeAccessFromImporter(Importer));

        if (!pImporter)
        {
            PathToMaterial = Core::AssetManager::GetPathToData() + g_PathToDataModels + _rFilename;

            Importer = Core::AssetImporter::AllocateAssimpImporter(PathToMaterial, Core::AssetImporter::SGeneratorFlag::Nothing);

            pImporter = static_cast<const Assimp::Importer*>(Core::AssetImporter::GetNativeAccessFromImporter(Importer));
        }

        if (!pImporter)
        {
            return;
        }

        const aiScene* pScene = pImporter->GetScene();

        // -----------------------------------------------------------------------------
        // Only single materials are currently supported!
        // Question: Do wee need multiple materials?
        // -----------------------------------------------------------------------------
        unsigned int NumberOfMaterials = pScene->mNumMaterials;

        if (static_cast<unsigned int>(_MaterialIndex) < NumberOfMaterials)
        {
            aiMaterial* pMaterial = pScene->mMaterials[_MaterialIndex];

            // -----------------------------------------------------------------------------
            // Fill data
            // -----------------------------------------------------------------------------
            FillMaterialFromAssimpMaterial(_pMaterial, pMaterial);
        }

        // -----------------------------------------------------------------------------
        // Release importer
        // -----------------------------------------------------------------------------
        Core::AssetImporter::ReleaseImporter(Importer);
    }

    // -----------------------------------------------------------------------------

    void CGfxMaterialManager::FillMaterialFromAssimpMaterial(CInternMaterial* _pComponent, const aiMaterial* _pAssimpMaterial)
    {
        assert(_pAssimpMaterial);

        // -----------------------------------------------------------------------------
        // Descriptor
        // -----------------------------------------------------------------------------
        SMaterialDescriptor MaterialDescriptor;

        MaterialDescriptor.m_pMaterialName = 0; 
        MaterialDescriptor.m_pColorMap     = 0; 
        MaterialDescriptor.m_pNormalMap    = 0; 
        MaterialDescriptor.m_pRoughnessMap = 0; 
        MaterialDescriptor.m_pMetalMaskMap = 0; 
        MaterialDescriptor.m_pAOMap        = 0; 
        MaterialDescriptor.m_pBumpMap      = 0; 
        MaterialDescriptor.m_Roughness     = 1.0f; 
        MaterialDescriptor.m_Reflectance   = 0.0f; 
        MaterialDescriptor.m_MetalMask     = 0.0f; 
        MaterialDescriptor.m_AlbedoColor   = glm::vec3(1.0f); 
        MaterialDescriptor.m_TilingOffset  = glm::vec4(1.0f, 1.0f, 0.0f, 0.0f);

        // -----------------------------------------------------------------------------
        // Values and textures
        // -----------------------------------------------------------------------------
        aiString  NativeString;
        aiColor4D DiffuseColor;

        if (_pAssimpMaterial->Get(AI_MATKEY_NAME, NativeString) == AI_SUCCESS)
        {
            MaterialDescriptor.m_pMaterialName = NativeString.data;
        }

        if (_pAssimpMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, DiffuseColor) == AI_SUCCESS)
        {
            MaterialDescriptor.m_AlbedoColor[0] = DiffuseColor.r;
            MaterialDescriptor.m_AlbedoColor[1] = DiffuseColor.g;
            MaterialDescriptor.m_AlbedoColor[2] = DiffuseColor.b;
        }

        if (_pAssimpMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &NativeString) == AI_SUCCESS)
        {
            MaterialDescriptor.m_pColorMap = NativeString.data;
        }

        if (_pAssimpMaterial->GetTexture(aiTextureType_HEIGHT, 0, &NativeString) == AI_SUCCESS)
        {
            MaterialDescriptor.m_pNormalMap = NativeString.data;
        }

        // -----------------------------------------------------------------------------
        // Check diffuse material if a *.mat file is set
        // -----------------------------------------------------------------------------
        if (MaterialDescriptor.m_pColorMap != nullptr)
        {
            std::string MaterialExaminer = MaterialDescriptor.m_pColorMap;

            if (MaterialExaminer.find(".mat") != std::string::npos)
            {
                FillMaterialFromXML(_pComponent, MaterialExaminer);

                return;
            }
        }

        // -----------------------------------------------------------------------------
        // Fill data
        // -----------------------------------------------------------------------------
        FillMaterialFromData(_pComponent, MaterialDescriptor);
    }

    // -----------------------------------------------------------------------------

    void CGfxMaterialManager::FillMaterialFromData(CInternMaterial* _pComponent, const SMaterialDescriptor& _rDescriptor)
    {
        // -----------------------------------------------------------------------------
        // Values
        // -----------------------------------------------------------------------------
        _pComponent->m_MaterialAttributes.m_Color        = _rDescriptor.m_AlbedoColor;
        _pComponent->m_MaterialAttributes.m_Roughness    = _rDescriptor.m_Roughness;
        _pComponent->m_MaterialAttributes.m_Reflectance  = _rDescriptor.m_Reflectance;
        _pComponent->m_MaterialAttributes.m_MetalMask    = _rDescriptor.m_MetalMask;
        _pComponent->m_MaterialAttributes.m_TilingOffset = _rDescriptor.m_TilingOffset;

        // -----------------------------------------------------------------------------
        // Textures
        // -----------------------------------------------------------------------------
        CTexturePtr TexturePtrs[CMaterial::SMaterialKey::s_NumberOfTextures];

        TexturePtrs[0] = 0;
        TexturePtrs[1] = 0;
        TexturePtrs[2] = 0;
        TexturePtrs[3] = 0;
        TexturePtrs[4] = 0;
        TexturePtrs[5] = 0;

        _pComponent->m_MaterialKey.m_HasDiffuseTex   = _rDescriptor.m_pColorMap != 0;
        _pComponent->m_MaterialKey.m_HasNormalTex    = _rDescriptor.m_pNormalMap != 0;
        _pComponent->m_MaterialKey.m_HasRoughnessTex = _rDescriptor.m_pRoughnessMap != 0;
        _pComponent->m_MaterialKey.m_HasMetallicTex  = _rDescriptor.m_pMetalMaskMap != 0;
        _pComponent->m_MaterialKey.m_HasAOTex        = _rDescriptor.m_pAOMap != 0;
        _pComponent->m_MaterialKey.m_HasBumpTex      = _rDescriptor.m_pBumpMap != 0;

        _pComponent->m_HasAlpha = false;
        _pComponent->m_HasBump = _pComponent->m_MaterialKey.m_HasBumpTex;

        const char* pColorMap     = _rDescriptor.m_pColorMap;
        const char* pNormalMap    = _rDescriptor.m_pNormalMap;
        const char* pRoughnessMap = _rDescriptor.m_pRoughnessMap;
        const char* pMetalMaskMap = _rDescriptor.m_pMetalMaskMap;
        const char* pAOMap        = _rDescriptor.m_pAOMap;
        const char* pBumpMap      = _rDescriptor.m_pBumpMap;

        STextureDescriptor TextureDescriptor;

        TextureDescriptor.m_NumberOfPixelsU  = STextureDescriptor::s_NumberOfPixelsFromSource;
        TextureDescriptor.m_NumberOfPixelsV  = STextureDescriptor::s_NumberOfPixelsFromSource;
        TextureDescriptor.m_NumberOfPixelsW  = STextureDescriptor::s_NumberOfPixelsFromSource;
        TextureDescriptor.m_NumberOfMipMaps  = STextureDescriptor::s_GenerateAllMipMaps;
        TextureDescriptor.m_NumberOfTextures = STextureDescriptor::s_NumberOfTexturesFromSource;
        TextureDescriptor.m_Binding          = CTexture::ShaderResource;
        TextureDescriptor.m_Access           = CTexture::CPUWrite;
        TextureDescriptor.m_Format           = CTexture::R8G8B8_UBYTE;
        TextureDescriptor.m_Usage            = CTexture::GPURead;
        TextureDescriptor.m_Semantic         = CTexture::Diffuse;
        TextureDescriptor.m_pPixels          = 0;
        TextureDescriptor.m_pFileName        = 0;

        if (_pComponent->m_MaterialKey.m_HasDiffuseTex)
        {
            TextureDescriptor.m_pFileName = pColorMap;

            TexturePtrs[0] = TextureManager::CreateTexture2D(TextureDescriptor);
        }

        if (_pComponent->m_MaterialKey.m_HasNormalTex)
        {
            TextureDescriptor.m_pFileName = pNormalMap;

            TexturePtrs[1] = TextureManager::CreateTexture2D(TextureDescriptor);
        }

        if (_pComponent->m_MaterialKey.m_HasRoughnessTex)
        {
            TextureDescriptor.m_pFileName = pRoughnessMap;

            TexturePtrs[2] = TextureManager::CreateTexture2D(TextureDescriptor);
        }

        if (_pComponent->m_MaterialKey.m_HasMetallicTex)
        {
            TextureDescriptor.m_pFileName = pMetalMaskMap;

            TexturePtrs[3] = TextureManager::CreateTexture2D(TextureDescriptor);
        }

        if (_pComponent->m_MaterialKey.m_HasAOTex)
        {
            TextureDescriptor.m_pFileName = pAOMap;

            TexturePtrs[4] = TextureManager::CreateTexture2D(TextureDescriptor);
        }

        if (_pComponent->m_MaterialKey.m_HasBumpTex)
        {
            TextureDescriptor.m_NumberOfPixelsW = 1;
            TextureDescriptor.m_Format = CTexture::R8_UBYTE;
            TextureDescriptor.m_pFileName = pBumpMap;

            TexturePtrs[5] = TextureManager::CreateTexture2D(TextureDescriptor);
        }

        _pComponent->m_TextureSetPtr = TextureManager::CreateTextureSet(TexturePtrs, CMaterial::SMaterialKey::s_NumberOfTextures);

        // -----------------------------------------------------------------------------
        // Sampler
        // -----------------------------------------------------------------------------
        CSamplerPtr SamplerPtrs[CMaterial::SMaterialKey::s_NumberOfTextures];

        SamplerPtrs[0] = SamplerManager::GetSampler(CSampler::MinMagMipLinearWrap);
        SamplerPtrs[1] = SamplerManager::GetSampler(CSampler::MinMagMipLinearWrap);
        SamplerPtrs[2] = SamplerManager::GetSampler(CSampler::MinMagMipLinearWrap);
        SamplerPtrs[3] = SamplerManager::GetSampler(CSampler::MinMagMipLinearWrap);
        SamplerPtrs[4] = SamplerManager::GetSampler(CSampler::MinMagMipLinearWrap);
        SamplerPtrs[5] = SamplerManager::GetSampler(CSampler::MinMagMipLinearWrap);

        _pComponent->m_SamplerSetPtr = SamplerManager::CreateSamplerSet(SamplerPtrs, CMaterial::SMaterialKey::s_NumberOfTextures);

        // -----------------------------------------------------------------------------
        // Shader
        // -----------------------------------------------------------------------------
        SetShaderOfMaterial(*_pComponent);
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

    CMaterialPtr CreateMaterialFromFile(const Base::Char* _pFilename, int _MaterialIndex)
    {
        return CGfxMaterialManager::GetInstance().CreateMaterialFromFile(_pFilename, _MaterialIndex);
    }

    // -----------------------------------------------------------------------------

    const CMaterialPtr GetDefaultMaterial()
    {
        return CGfxMaterialManager::GetInstance().GetDefaultMaterial();
    }
} // namespace MaterialManager
} // namespace Gfx