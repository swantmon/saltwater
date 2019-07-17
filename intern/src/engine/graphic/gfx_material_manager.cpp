
#include "engine/engine_precompiled.h"

#include "base/base_crc.h"
#include "base/base_exception.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "engine/core/core_console.h"
#include "engine/core/core_asset_importer.h"
#include "engine/core/core_asset_manager.h"

#include "engine/data/data_component.h"
#include "engine/data/data_component_manager.h"
#include "engine/data/data_entity.h"
#include "engine/data/data_material_component.h"

#include "engine/graphic/gfx_material.h"
#include "engine/graphic/gfx_material_manager.h"
#include "engine/graphic/gfx_texture_manager.h"
#include "engine/graphic/gfx_sampler_manager.h"
#include "engine/graphic/gfx_shader.h"
#include "engine/graphic/gfx_shader_manager.h"

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
        "material/tcs_test.glsl",
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
        "material/tes_test.glsl",
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
        "material/gs_test.glsl",
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
        "material/fs_material.glsl",
    };

    const char* g_pShaderNamesPS[] =
    {
        "PSShaderMaterialDisney",
    };

    const char* g_pForwardShaderFilenamePS[] =
    {
        "material/fs_material_forward.glsl",
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
        "#define USE_TEX_ALPHA\n"    ,
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

        CMaterialPtr CreateMaterialFromName(const std::string& _rMaterialname, Dt::CMaterialComponent* _pComponent);

        CMaterialPtr CreateMaterialFromXML(const std::string& _rPathToFile, Dt::CMaterialComponent* _pComponent);

        CMaterialPtr CreateMaterialFromAssimp(const std::string& _rPathToFile, int _MaterialIndex, Dt::CMaterialComponent* _pComponent);

        CMaterialPtr GetMaterialByHash(const Dt::CMaterialComponent::BHash _Hash);

        const CMaterialPtr GetDefaultMaterial();

    private:

        struct SMaterialDescriptor
        {
            std::string m_MaterialName;
            std::string m_ColorTexture;
            std::string m_NormalTexture;
            std::string m_RoughnessTexture;
            std::string m_MetalTexture;
            std::string m_AmbientOcclusionTexture;
            std::string m_BumpTexture;
            std::string m_AlphaTexture;
            float       m_Roughness;
            float       m_Reflectance;
            float       m_MetalMask;
            float       m_Displacement;
            float       m_RefractionIndex;
            glm::vec4   m_AlbedoColor;
            glm::vec4   m_TilingOffset;
        };

    private:

        class CInternMaterial : public CMaterial
        {
        private:

            friend class CGfxMaterialManager;
        };

    private:

        using CMaterials = Base::CManagedPool<CInternMaterial, 32, 1>;
        using CMaterialsByHash = std::unordered_map<Base::BHash, CInternMaterial*>;

    private:

        static const SMaterialDescriptor s_DefaultDescriptor;

    private:

        CMaterials       m_Materials;
        CMaterialsByHash m_MaterialsByHash;
        CMaterialPtr     m_DefaultMaterialPtr;

        Dt::CComponentManager::CComponentDelegate::HandleType m_OnDirtyComponentDelegate;

    private:

        void OnDirtyComponent(Dt::IComponent* _pComponent);

        CInternMaterial* CreateMaterialFromDescription(const SMaterialDescriptor& _rDescriptor, Dt::CMaterialComponent* _pComponent = nullptr);

        void FillMaterialFromData(CInternMaterial* _pMaterial, const SMaterialDescriptor& _rDescription);

        void SetShaderOfMaterial(CInternMaterial& _rMaterial) const;
    };
} // namespace

namespace
{
    const CGfxMaterialManager::SMaterialDescriptor CGfxMaterialManager::s_DefaultDescriptor =
    {
        "STATIC CONST DEFAULT MATERIAL: default.mat", "", "", "", "", "", "", "",
        1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        glm::vec4(glm::vec3(0.8f), 1.0f),
        glm::vec4(1.0f, 1.0f, 0.0f, 0.0f)
    };
} // namespace 

namespace
{
    CGfxMaterialManager::CGfxMaterialManager()
        : m_Materials         ( )
        , m_MaterialsByHash   ( )
        , m_DefaultMaterialPtr(nullptr)
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
        // Default
        // -----------------------------------------------------------------------------
        m_DefaultMaterialPtr = CreateMaterialFromDescription(s_DefaultDescriptor);

        // -----------------------------------------------------------------------------
        // Set dirty handler of data textures
        // -----------------------------------------------------------------------------
        m_OnDirtyComponentDelegate = Dt::CComponentManager::GetInstance().RegisterDirtyComponentHandler(std::bind(&CGfxMaterialManager::OnDirtyComponent, this, std::placeholders::_1));
    }

    // -----------------------------------------------------------------------------

    void CGfxMaterialManager::OnExit()
    {
        m_DefaultMaterialPtr = nullptr;

        m_MaterialsByHash.clear();

        m_Materials.Clear();
    }

    // -----------------------------------------------------------------------------

    CMaterialPtr CGfxMaterialManager::CreateMaterialFromName(const std::string& _rMaterialname, Dt::CMaterialComponent* _pComponent)
    {
        // -----------------------------------------------------------------------------
        // Hash
        // -----------------------------------------------------------------------------
        SMaterialDescriptor MaterialDescriptor = s_DefaultDescriptor;

        MaterialDescriptor.m_MaterialName = _rMaterialname;

        return CreateMaterialFromDescription(MaterialDescriptor, _pComponent);
    }

    // -----------------------------------------------------------------------------

    CMaterialPtr CGfxMaterialManager::CreateMaterialFromXML(const std::string& _rPathToFile, Dt::CMaterialComponent* _pComponent)
    {
        // -----------------------------------------------------------------------------
        // Descriptor
        // -----------------------------------------------------------------------------
        SMaterialDescriptor MaterialDescriptor = s_DefaultDescriptor;

        // -----------------------------------------------------------------------------
        // Load data
        // -----------------------------------------------------------------------------
        std::string PathToAsset = Core::AssetManager::GetPathToAssets() + "/" + _rPathToFile;

        auto Importer = Core::AssetImporter::AllocateTinyXMLImporter(PathToAsset);

        if (Importer == nullptr) return m_DefaultMaterialPtr;

        auto* pMaterialFile = static_cast<tinyxml2::XMLDocument*>(Core::AssetImporter::GetNativeAccessFromImporter(Importer));

        if (pMaterialFile == nullptr) return m_DefaultMaterialPtr;

        tinyxml2::XMLElement* pElementDefinition  = pMaterialFile->FirstChildElement("MaterialDefinition");
        tinyxml2::XMLElement* pElementColor       = pElementDefinition->FirstChildElement("Color");
        tinyxml2::XMLElement* pElementNormal      = pElementDefinition->FirstChildElement("Normal");
        tinyxml2::XMLElement* pElementRoughness   = pElementDefinition->FirstChildElement("Roughness");
        tinyxml2::XMLElement* pElementReflectance = pElementDefinition->FirstChildElement("Reflectance");
        tinyxml2::XMLElement* pElementMetallic    = pElementDefinition->FirstChildElement("Metallic");
        tinyxml2::XMLElement* pElementRefraction  = pElementDefinition->FirstChildElement("Refraction");
        tinyxml2::XMLElement* pElementAO          = pElementDefinition->FirstChildElement("AO");
        tinyxml2::XMLElement* pElementBump        = pElementDefinition->FirstChildElement("Bump");
        tinyxml2::XMLElement* pElementAlpha       = pElementDefinition->FirstChildElement("Alpha");
        tinyxml2::XMLElement* pElementTiling      = pElementDefinition->FirstChildElement("Tiling");
        tinyxml2::XMLElement* pElementOffset      = pElementDefinition->FirstChildElement("Offset");

        // -----------------------------------------------------------------------------
        // Values
        // -----------------------------------------------------------------------------
        MaterialDescriptor.m_MaterialName = std::string(pElementDefinition->Attribute("Name")) + ": " + _rPathToFile;

        if (pElementColor != nullptr)
        {
            float ColorR = pElementColor->FloatAttribute("R", 0.0f);
            float ColorG = pElementColor->FloatAttribute("G", 0.0f);
            float ColorB = pElementColor->FloatAttribute("B", 0.0f);
            float ColorA = pElementColor->FloatAttribute("A", 1.0f);

            MaterialDescriptor.m_AlbedoColor = glm::vec4(ColorR, ColorG, ColorB, ColorA);
        }

        if (pElementRoughness) MaterialDescriptor.m_Roughness = pElementRoughness->FloatAttribute("V");
        if (pElementReflectance) MaterialDescriptor.m_Reflectance = pElementReflectance->FloatAttribute("V");
        if (pElementMetallic) MaterialDescriptor.m_MetalMask = pElementMetallic->FloatAttribute("V");
        if (pElementBump) MaterialDescriptor.m_Displacement = pElementBump->FloatAttribute("V");
        if (pElementAlpha) MaterialDescriptor.m_AlbedoColor[3] = pElementAlpha->FloatAttribute("V", MaterialDescriptor.m_AlbedoColor[3]);
        if (pElementRefraction) MaterialDescriptor.m_RefractionIndex = pElementRefraction->FloatAttribute("V", 1.0f);

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
        if (pElementColor != nullptr && pElementColor->Attribute("Map")) MaterialDescriptor.m_ColorTexture = pElementColor->Attribute("Map");
        if (pElementNormal != nullptr && pElementNormal->Attribute("Map")) MaterialDescriptor.m_NormalTexture = pElementNormal->Attribute("Map");
        if (pElementRoughness != nullptr && pElementRoughness->Attribute("Map")) MaterialDescriptor.m_RoughnessTexture = pElementRoughness->Attribute("Map");
        if (pElementMetallic != nullptr && pElementMetallic->Attribute("Map")) MaterialDescriptor.m_MetalTexture = pElementMetallic->Attribute("Map");
        if (pElementBump != nullptr && pElementBump->Attribute("Map")) MaterialDescriptor.m_BumpTexture = pElementBump->Attribute("Map");
        if (pElementAO != nullptr && pElementAO->Attribute("Map")) MaterialDescriptor.m_AmbientOcclusionTexture = pElementAO->Attribute("Map");
        if (pElementAlpha != nullptr && pElementAlpha->Attribute("Map")) MaterialDescriptor.m_AlphaTexture = pElementAlpha->Attribute("Map");

        return CreateMaterialFromDescription(MaterialDescriptor, _pComponent);
    }

    // -----------------------------------------------------------------------------

    CMaterialPtr CGfxMaterialManager::CreateMaterialFromAssimp(const std::string& _rPathToFile, int _MaterialIndex, Dt::CMaterialComponent* _pComponent)
    {
        // -----------------------------------------------------------------------------
        // Descriptor
        // -----------------------------------------------------------------------------
        SMaterialDescriptor MaterialDescriptor = s_DefaultDescriptor;

        // -----------------------------------------------------------------------------
        // Importer
        // -----------------------------------------------------------------------------
        std::string PathToAsset = Core::AssetManager::GetPathToAssets() + "/" + _rPathToFile;

        auto Importer = Core::AssetImporter::AllocateAssimpImporter(PathToAsset, Core::AssetGenerator::SGeneratorFlag::Nothing);

        if (Importer == nullptr) return m_DefaultMaterialPtr;

        const auto* pImporter = static_cast<const Assimp::Importer*>(Core::AssetImporter::GetNativeAccessFromImporter(Importer));

        if (pImporter == nullptr) return m_DefaultMaterialPtr;

        const aiScene* pScene = pImporter->GetScene();

        if (pScene == nullptr) return m_DefaultMaterialPtr;

        // -----------------------------------------------------------------------------
        // Only single materials are currently supported!
        // Question: Do wee need multiple materials?
        // -----------------------------------------------------------------------------
        unsigned int NumberOfMaterials = pScene->mNumMaterials;

        if (static_cast<unsigned int>(_MaterialIndex) < NumberOfMaterials)
        {
            aiMaterial* pMaterial = pScene->mMaterials[_MaterialIndex];

            // -----------------------------------------------------------------------------
            // Values and textures
            // -----------------------------------------------------------------------------
            aiString  NativeString;
            aiColor4D DiffuseColor;

            if (pMaterial->Get(AI_MATKEY_NAME, NativeString) == AI_SUCCESS)
            {
                MaterialDescriptor.m_MaterialName = std::string(NativeString.data) + ": " + _rPathToFile;
            }

            if (pMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, DiffuseColor) == AI_SUCCESS)
            {
                MaterialDescriptor.m_AlbedoColor[0] = DiffuseColor.r;
                MaterialDescriptor.m_AlbedoColor[1] = DiffuseColor.g;
                MaterialDescriptor.m_AlbedoColor[2] = DiffuseColor.b;
            }

            if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &NativeString) == AI_SUCCESS)
            {
                MaterialDescriptor.m_ColorTexture = NativeString.data;
            }

            if (pMaterial->GetTexture(aiTextureType_HEIGHT, 0, &NativeString) == AI_SUCCESS)
            {
                MaterialDescriptor.m_NormalTexture = NativeString.data;
            }

            if (pMaterial->GetTexture(aiTextureType_OPACITY, 0, &NativeString) == AI_SUCCESS)
            {
                MaterialDescriptor.m_AlphaTexture = NativeString.data;
            }

            // -----------------------------------------------------------------------------
            // Check diffuse material if a *.mat file is set
            // -----------------------------------------------------------------------------
            if (MaterialDescriptor.m_ColorTexture.length() > 0)
            {
                if (MaterialDescriptor.m_ColorTexture.find(".mat") != std::string::npos)
                {
                    std::string PathToMat = MaterialDescriptor.m_ColorTexture;

                    return CreateMaterialFromXML(PathToMat, _pComponent);
                }
            }
        }

        return CreateMaterialFromDescription(MaterialDescriptor, _pComponent);
    }

    // -----------------------------------------------------------------------------

    CMaterialPtr CGfxMaterialManager::GetMaterialByHash(const Dt::CMaterialComponent::BHash _Hash)
    {
        if (m_MaterialsByHash.find(_Hash) == m_MaterialsByHash.end()) return m_DefaultMaterialPtr;

        return m_MaterialsByHash.at(_Hash);
    }

    // -----------------------------------------------------------------------------

    const CMaterialPtr CGfxMaterialManager::GetDefaultMaterial()
    {
        return m_DefaultMaterialPtr;
    }

    // -----------------------------------------------------------------------------

    void CGfxMaterialManager::OnDirtyComponent(Dt::IComponent* _pComponent)
    {
        if (_pComponent->GetTypeInfo() != Base::CTypeInfo::Get<Dt::CMaterialComponent>()) return;

        auto* pMaterialComponent = static_cast<Dt::CMaterialComponent*>(_pComponent);

        // -----------------------------------------------------------------------------
        // Dirty check
        // -----------------------------------------------------------------------------
        unsigned int DirtyFlags;

        DirtyFlags = pMaterialComponent->GetDirtyFlags();

        // -----------------------------------------------------------------------------
        // Material
        // -----------------------------------------------------------------------------
        SMaterialDescriptor MaterialDescriptor;

        MaterialDescriptor.m_MaterialName = pMaterialComponent->GetMaterialname();

        MaterialDescriptor.m_Roughness       = pMaterialComponent->GetRoughness();
        MaterialDescriptor.m_Reflectance     = pMaterialComponent->GetReflectance();
        MaterialDescriptor.m_MetalMask       = pMaterialComponent->GetMetalness();
        MaterialDescriptor.m_Displacement    = pMaterialComponent->GetDisplacement();
        MaterialDescriptor.m_RefractionIndex = pMaterialComponent->GetRefractionIndex();
        MaterialDescriptor.m_AlbedoColor     = pMaterialComponent->GetColor();
        MaterialDescriptor.m_TilingOffset    = pMaterialComponent->GetTilingOffset();

        MaterialDescriptor.m_ColorTexture            = pMaterialComponent->GetColorTexture();
        MaterialDescriptor.m_NormalTexture           = pMaterialComponent->GetNormalTexture();
        MaterialDescriptor.m_RoughnessTexture        = pMaterialComponent->GetRoughnessTexture();
        MaterialDescriptor.m_MetalTexture            = pMaterialComponent->GetMetalTexture();
        MaterialDescriptor.m_AmbientOcclusionTexture = pMaterialComponent->GetAmbientOcclusionTexture();
        MaterialDescriptor.m_BumpTexture             = pMaterialComponent->GetBumpTexture();
        MaterialDescriptor.m_AlphaTexture            = pMaterialComponent->GetAlphaTexture();

        CInternMaterial* pInternMaterial = nullptr;

        if ((DirtyFlags & Dt::CMaterialComponent::DirtyCreate) != 0)
        {
            pInternMaterial = static_cast<CInternMaterial*>(CreateMaterialFromDescription(MaterialDescriptor));

            pMaterialComponent->SetFacet(Dt::CMaterialComponent::Graphic, pInternMaterial);
        }
        else if ((DirtyFlags & Dt::CMaterialComponent::DirtyInfo) != 0)
        {
            pInternMaterial = static_cast<CInternMaterial*>(pMaterialComponent->GetFacet(Dt::CMaterialComponent::Graphic));

            FillMaterialFromData(pInternMaterial, MaterialDescriptor);
        }
    }

    // -----------------------------------------------------------------------------

    CGfxMaterialManager::CInternMaterial* CGfxMaterialManager::CreateMaterialFromDescription(const SMaterialDescriptor& _rDescriptor, Dt::CMaterialComponent* _pComponent)
    {
        // -----------------------------------------------------------------------------
        // Hash
        // -----------------------------------------------------------------------------
        auto Hash = (Base::BHash)(0);

        if (_rDescriptor.m_MaterialName.size() > 0)
        {
             Hash = Base::CRC32(_rDescriptor.m_MaterialName.c_str(), static_cast<unsigned int>(_rDescriptor.m_MaterialName.length()));

            if (m_MaterialsByHash.find(Hash) != m_MaterialsByHash.end()) return m_MaterialsByHash.at(Hash);
        }

        // -----------------------------------------------------------------------------
        // Fill material component (if needed)
        // -----------------------------------------------------------------------------
        if (_pComponent != nullptr)
        {
            _pComponent->SetMaterialname(_rDescriptor.m_MaterialName);

            _pComponent->SetRoughness(_rDescriptor.m_Roughness);
            _pComponent->SetReflectance(_rDescriptor.m_Reflectance);
            _pComponent->SetMetalness(_rDescriptor.m_MetalMask);
            _pComponent->SetDisplacement(_rDescriptor.m_Displacement);
            _pComponent->SetColor(_rDescriptor.m_AlbedoColor);
            _pComponent->SetTilingOffset(_rDescriptor.m_TilingOffset);

            _pComponent->SetColorTexture(_rDescriptor.m_ColorTexture);
            _pComponent->SetNormalTexture(_rDescriptor.m_NormalTexture);
            _pComponent->SetRoughnessTexture(_rDescriptor.m_RoughnessTexture);
            _pComponent->SetMetalTexture(_rDescriptor.m_MetalTexture);
            _pComponent->SetAmbientOcclusionTexture(_rDescriptor.m_AmbientOcclusionTexture);
            _pComponent->SetBumpTexture(_rDescriptor.m_BumpTexture);
            _pComponent->SetAlphaTexture(_rDescriptor.m_AlphaTexture);
        }

        // -----------------------------------------------------------------------------
        // Create material
        // -----------------------------------------------------------------------------
        auto Component = m_Materials.Allocate();

        FillMaterialFromData(Component, _rDescriptor);

        if (Hash != 0) m_MaterialsByHash[Hash] = Component;

        return Component;
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
        // Refraction values
        // -----------------------------------------------------------------------------
        _pComponent->m_MaterialRefractionAttributes.m_IndexOfRefraction = _rDescriptor.m_RefractionIndex;

        // -----------------------------------------------------------------------------
        // Textures
        // -----------------------------------------------------------------------------
        CTexturePtr TexturePtrs[CMaterial::SMaterialKey::s_NumberOfTextures];

        TexturePtrs[0] = nullptr;
        TexturePtrs[1] = nullptr;
        TexturePtrs[2] = nullptr;
        TexturePtrs[3] = nullptr;
        TexturePtrs[4] = nullptr;
        TexturePtrs[5] = nullptr;
        TexturePtrs[6] = nullptr;

        _pComponent->m_MaterialKey.m_HasDiffuseTex   = _rDescriptor.m_ColorTexture.length() > 0;
        _pComponent->m_MaterialKey.m_HasNormalTex    = _rDescriptor.m_NormalTexture.length() > 0;
        _pComponent->m_MaterialKey.m_HasRoughnessTex = _rDescriptor.m_RoughnessTexture.length() > 0;
        _pComponent->m_MaterialKey.m_HasMetallicTex  = _rDescriptor.m_MetalTexture.length() > 0;
        _pComponent->m_MaterialKey.m_HasAOTex        = _rDescriptor.m_AmbientOcclusionTexture.length() > 0;
        _pComponent->m_MaterialKey.m_HasBumpTex      = _rDescriptor.m_BumpTexture.length() > 0;
        _pComponent->m_MaterialKey.m_HasAlphaTex     = _rDescriptor.m_AlphaTexture.length() > 0;

        const char* pColorMap     = _rDescriptor.m_ColorTexture.c_str();
        const char* pNormalMap    = _rDescriptor.m_NormalTexture.c_str();
        const char* pRoughnessMap = _rDescriptor.m_RoughnessTexture.c_str();
        const char* pMetalMaskMap = _rDescriptor.m_MetalTexture.c_str();
        const char* pAOMap        = _rDescriptor.m_AmbientOcclusionTexture.c_str();
        const char* pBumpMap      = _rDescriptor.m_BumpTexture.c_str();
        const char* pAlphaMap     = _rDescriptor.m_AlphaTexture.c_str();

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
        TextureDescriptor.m_pPixels          = nullptr;
        TextureDescriptor.m_pFileName        = nullptr;

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
            TextureDescriptor.m_NumberOfPixelsW = 1;
            TextureDescriptor.m_Format          = CTexture::R8G8B8_UBYTE;
            TextureDescriptor.m_pFileName       = pRoughnessMap;

            TexturePtrs[2] = TextureManager::CreateTexture2D(TextureDescriptor);
        }

        if (_pComponent->m_MaterialKey.m_HasMetallicTex)
        {
            TextureDescriptor.m_NumberOfPixelsW = 1;
            TextureDescriptor.m_Format          = CTexture::R8G8B8_UBYTE;
            TextureDescriptor.m_pFileName       = pMetalMaskMap;

            TexturePtrs[3] = TextureManager::CreateTexture2D(TextureDescriptor);
        }

        if (_pComponent->m_MaterialKey.m_HasAOTex)
        {
            TextureDescriptor.m_NumberOfPixelsW = 1;
            TextureDescriptor.m_Format          = CTexture::R8G8B8_UBYTE;
            TextureDescriptor.m_pFileName       = pAOMap;

            TexturePtrs[4] = TextureManager::CreateTexture2D(TextureDescriptor);
        }

        if (_pComponent->m_MaterialKey.m_HasBumpTex)
        {
            TextureDescriptor.m_NumberOfPixelsW = 1;
            TextureDescriptor.m_Format          = CTexture::R8_UBYTE;
            TextureDescriptor.m_pFileName       = pBumpMap;

            TexturePtrs[5] = TextureManager::CreateTexture2D(TextureDescriptor);
        }

        if (_pComponent->m_MaterialKey.m_HasAlphaTex)
        {
            TextureDescriptor.m_NumberOfPixelsW = 1;
            TextureDescriptor.m_Format          = CTexture::R8_UBYTE;
            TextureDescriptor.m_pFileName       = pAlphaMap;

            TexturePtrs[6] = TextureManager::CreateTexture2D(TextureDescriptor);
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
        SamplerPtrs[6] = SamplerManager::GetSampler(CSampler::MinMagMipLinearWrap);

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
        if (_rMaterial.m_MaterialKey.m_HasAlphaTex)     Defines += g_pShaderAttributeDefines[6];

        const char* pDefineString = nullptr;

        if (Defines.length() > 0)
        {
            pDefineString = Defines.c_str();
        }

        // -----------------------------------------------------------------------------
        // Bump Mapping
        // -----------------------------------------------------------------------------
        if (_rMaterial.HasBump())
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

    CMaterialPtr CreateMaterialFromName(const std::string& _rMaterialname, Dt::CMaterialComponent* _pComponent)
    {
        return CGfxMaterialManager::GetInstance().CreateMaterialFromName(_rMaterialname, _pComponent);
    }

    // -----------------------------------------------------------------------------

    CMaterialPtr CreateMaterialFromXML(const std::string& _rPathToFile, Dt::CMaterialComponent* _pComponent)
    {
        return CGfxMaterialManager::GetInstance().CreateMaterialFromXML(_rPathToFile, _pComponent);
    }

    // -----------------------------------------------------------------------------

    CMaterialPtr CreateMaterialFromAssimp(const std::string& _rPathToFile, int _MaterialIndex, Dt::CMaterialComponent* _pComponent)
    {
        return CGfxMaterialManager::GetInstance().CreateMaterialFromAssimp(_rPathToFile, _MaterialIndex, _pComponent);
    }

    // -----------------------------------------------------------------------------

    CMaterialPtr GetMaterialByHash(const Dt::CMaterialComponent::BHash _Hash)
    {
        return CGfxMaterialManager::GetInstance().GetMaterialByHash(_Hash);
    }

    // -----------------------------------------------------------------------------

    const CMaterialPtr GetDefaultMaterial()
    {
        return CGfxMaterialManager::GetInstance().GetDefaultMaterial();
    }
} // namespace MaterialManager
} // namespace Gfx