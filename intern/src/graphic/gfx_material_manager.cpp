
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

        CMaterialPtr CreateMaterial(const Core::MaterialImporter::SMaterialDescriptor& _rDescriptor);

        const CMaterialPtr GetDefaultMaterial();

    private:

        class CInternMaterial : public CMaterial
        {
        private:

            friend class CGfxMaterialManager;
        };

    private:

        typedef Base::CManagedPool<CInternMaterial, 32, 1> CMaterials;
        typedef std::map<Base::BHash, CInternMaterial*> CMaterialsByHash;

    private:

        CMaterials       m_Materials;
        CMaterialsByHash m_MaterialsByHash;
        CMaterialPtr     m_DefaultMaterialPtr;

    private:

        void OnDirtyComponent(Base::IComponent* _pComponent);

        void FillMaterialFromData(CInternMaterial* _pMaterial, const Core::MaterialImporter::SMaterialDescriptor& _rDescription);

        void SetShaderOfMaterial(CInternMaterial& _rMaterial) const;
    };
} // namespace

namespace
{
    CGfxMaterialManager::CGfxMaterialManager()
        : m_Materials         ( )
        , m_MaterialsByHash   ( )
        , m_DefaultMaterialPtr(0)
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
        Core::MaterialImporter::SMaterialDescriptor MaterialDescriptor;

        MaterialDescriptor.m_MaterialName            = "STATIC CONST DEFAULT GFX MATERIAL: default.mat"; 
        MaterialDescriptor.m_ColorTexture            = ""; 
        MaterialDescriptor.m_NormalTexture           = ""; 
        MaterialDescriptor.m_RoughnessTexture        = ""; 
        MaterialDescriptor.m_MetalTexture            = ""; 
        MaterialDescriptor.m_AmbientOcclusionTexture = ""; 
        MaterialDescriptor.m_BumpTexture             = ""; 
        MaterialDescriptor.m_Roughness               = 1.0f; 
        MaterialDescriptor.m_Reflectance             = 0.0f; 
        MaterialDescriptor.m_MetalMask               = 0.0f; 
        MaterialDescriptor.m_AlbedoColor             = glm::vec3(0.8f); 
        MaterialDescriptor.m_TilingOffset            = glm::vec4(1.0f, 1.0f, 0.0f, 0.0f);

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

        m_MaterialsByHash.clear();

        m_Materials.Clear();
    }

    // -----------------------------------------------------------------------------

    CMaterialPtr CGfxMaterialManager::CreateMaterial(const Core::MaterialImporter::SMaterialDescriptor& _rDescriptor)
    {
        // -----------------------------------------------------------------------------
        // Hash
        // -----------------------------------------------------------------------------
        Base::BHash Hash = Base::CRC32(_rDescriptor.m_MaterialName.c_str(), _rDescriptor.m_MaterialName.length());

        if (m_MaterialsByHash.find(Hash) != m_MaterialsByHash.end())
        {
            return m_MaterialsByHash.at(Hash);
        }

        // -----------------------------------------------------------------------------
        // New material
        // -----------------------------------------------------------------------------
        auto pMaterial = m_Materials.Allocate();

        FillMaterialFromData(pMaterial, _rDescriptor);

        // -----------------------------------------------------------------------------
        // Add material to hash table
        // -----------------------------------------------------------------------------
        m_MaterialsByHash[Hash] = pMaterial;

        return CMaterialPtr(pMaterial);
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
        Core::MaterialImporter::SMaterialDescriptor MaterialDescriptor;

        MaterialDescriptor.m_MaterialName = pMaterialComponent->GetMaterialname();

        MaterialDescriptor.m_Roughness    = pMaterialComponent->GetRoughness();
        MaterialDescriptor.m_Reflectance  = pMaterialComponent->GetReflectance();
        MaterialDescriptor.m_MetalMask    = pMaterialComponent->GetMetalness();
        MaterialDescriptor.m_Displacement = pMaterialComponent->GetDisplacement();
        MaterialDescriptor.m_AlbedoColor  = pMaterialComponent->GetColor();
        MaterialDescriptor.m_TilingOffset = pMaterialComponent->GetTilingOffset();

        MaterialDescriptor.m_ColorTexture            = pMaterialComponent->GetColorTexture();
        MaterialDescriptor.m_NormalTexture           = pMaterialComponent->GetNormalTexture();
        MaterialDescriptor.m_RoughnessTexture        = pMaterialComponent->GetRoughnessTexture();
        MaterialDescriptor.m_MetalTexture            = pMaterialComponent->GetMetalTexture();
        MaterialDescriptor.m_AmbientOcclusionTexture = pMaterialComponent->GetAmbientOcclusionTexture();
        MaterialDescriptor.m_BumpTexture             = pMaterialComponent->GetBumpTexture();

        CInternMaterial* pInternMaterial = 0;

        if ((DirtyFlags & Dt::CMaterialComponent::DirtyCreate) != 0)
        {
            pInternMaterial = static_cast<CInternMaterial*>(CreateMaterial(MaterialDescriptor).GetPtr());

            pMaterialComponent->SetFacet(Dt::CMaterialComponent::Graphic, pInternMaterial);
        }
        else if ((DirtyFlags & Dt::CMaterialComponent::DirtyInfo) != 0)
        {
            pInternMaterial = static_cast<CInternMaterial*>(pMaterialComponent->GetFacet(Dt::CMaterialComponent::Graphic));
        }

        if (pInternMaterial)
        {
            FillMaterialFromData(pInternMaterial, MaterialDescriptor);
        }
    }

    // -----------------------------------------------------------------------------

    void CGfxMaterialManager::FillMaterialFromData(CInternMaterial* _pComponent, const Core::MaterialImporter::SMaterialDescriptor& _rDescriptor)
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

        _pComponent->m_MaterialKey.m_HasDiffuseTex   = _rDescriptor.m_ColorTexture.length() > 0;
        _pComponent->m_MaterialKey.m_HasNormalTex    = _rDescriptor.m_NormalTexture.length() > 0;
        _pComponent->m_MaterialKey.m_HasRoughnessTex = _rDescriptor.m_RoughnessTexture.length() > 0;
        _pComponent->m_MaterialKey.m_HasMetallicTex  = _rDescriptor.m_MetalTexture.length() > 0;
        _pComponent->m_MaterialKey.m_HasAOTex        = _rDescriptor.m_AmbientOcclusionTexture.length() > 0;
        _pComponent->m_MaterialKey.m_HasBumpTex      = _rDescriptor.m_BumpTexture.length() > 0;

        _pComponent->m_HasAlpha = false;
        _pComponent->m_HasBump = _pComponent->m_MaterialKey.m_HasBumpTex;

        const char* pColorMap     = _rDescriptor.m_ColorTexture.c_str();
        const char* pNormalMap    = _rDescriptor.m_NormalTexture.c_str();
        const char* pRoughnessMap = _rDescriptor.m_RoughnessTexture.c_str();
        const char* pMetalMaskMap = _rDescriptor.m_MetalTexture.c_str();
        const char* pAOMap        = _rDescriptor.m_AmbientOcclusionTexture.c_str();
        const char* pBumpMap      = _rDescriptor.m_BumpTexture.c_str();

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

    const CMaterialPtr GetDefaultMaterial()
    {
        return CGfxMaterialManager::GetInstance().GetDefaultMaterial();
    }
} // namespace MaterialManager
} // namespace Gfx