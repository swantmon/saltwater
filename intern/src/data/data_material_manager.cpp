
#include "data/data_precompiled.h"

#include "base/base_aabb3.h"
#include "base/base_console.h"
#include "base/base_crc.h"
#include "base/base_exception.h"
#include "base/base_include_glm.h"
#include "base/base_memory.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"
#include "base/base_pool.h"

#include "core/core_asset_manager.h"
#include "core/core_time.h"

#include "data/data_entity.h"
#include "data/data_map.h"
#include "data/data_material.h"
#include "data/data_material_manager.h"
#include "data/data_texture_2d.h"
#include "data/data_texture_manager.h"

#include <unordered_map>
#include <vector>

using namespace Dt;
using namespace Dt::MaterialManager;

namespace
{
    class CDtMaterialManager : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CDtMaterialManager)
        
    public:
        
        CDtMaterialManager();
        ~CDtMaterialManager();
        
    public:

        void OnStart();
        void OnExit();

        CMaterial& CreateMaterial(const SMaterialDescriptor& _rDescriptor);

        CMaterial& GetDefaultMaterial();

        CMaterial& GetMaterialByHash(unsigned int _Hash);
        
    private:
        
        class CInternMaterial : public CMaterial
        {
        private:
            
            friend class CDtMaterialManager;
        };
        
    private:
        
        typedef Base::CPool<CInternMaterial, 1024> CMaterials;

        typedef std::unordered_map<unsigned int, CInternMaterial*> CMaterialByHashs;
        
    private:
        
        CMaterials       m_Materials;
        CMaterialByHashs m_MaterialByHashs;

        CInternMaterial* m_pDefaultMaterial;

    private:

        void FreeMaterial(CInternMaterial& _rMaterial);
    };
} // namespace

namespace
{
    CDtMaterialManager::CDtMaterialManager()
        : m_Materials       ()
        , m_MaterialByHashs ()
        , m_pDefaultMaterial(0)
    {
    }
    
    // -----------------------------------------------------------------------------
    
    CDtMaterialManager::~CDtMaterialManager()
    {
    }

    // -----------------------------------------------------------------------------

    void CDtMaterialManager::OnStart()
    {
        SMaterialDescriptor MaterialDescriptor;

        MaterialDescriptor.m_pMaterialName   = "STATIC DEFAULT MATERIAL";
        MaterialDescriptor.m_pColorMap       = "";
        MaterialDescriptor.m_pNormalMap      = "";
        MaterialDescriptor.m_pRoughnessMap   = "";
        MaterialDescriptor.m_pMetalMaskMap   = "";
        MaterialDescriptor.m_pAOMap          = "";
        MaterialDescriptor.m_pBumpMap        = "";
        MaterialDescriptor.m_Roughness       = 1.0f;
        MaterialDescriptor.m_Reflectance     = 0.0f;
        MaterialDescriptor.m_MetalMask       = 0.0f;
        MaterialDescriptor.m_Displacement    = 0.0f;
        MaterialDescriptor.m_AlbedoColor     = glm::vec3(0.8f);
        MaterialDescriptor.m_TilingOffset    = glm::vec4(0.0f);
        MaterialDescriptor.m_pFileName       = "";

        m_pDefaultMaterial = &static_cast<CInternMaterial&>(CreateMaterial(MaterialDescriptor));
    }

    // -----------------------------------------------------------------------------

    void CDtMaterialManager::OnExit()
    {
        m_Materials.Free(m_pDefaultMaterial);

        m_pDefaultMaterial = 0;

        // -----------------------------------------------------------------------------

        m_Materials.Clear();

        m_MaterialByHashs.clear();
    }
    
    // -----------------------------------------------------------------------------
    
    CMaterial& CDtMaterialManager::CreateMaterial(const SMaterialDescriptor& _rDescriptor)
    {
        // -----------------------------------------------------------------------------
        // Create hash value
        // -----------------------------------------------------------------------------
        unsigned int Hash = 0;

        if (_rDescriptor.m_pFileName.length() > 0)
        {
            int NumberOfBytes = static_cast<unsigned int>(_rDescriptor.m_pFileName.length() * sizeof(char));
            const void* pData = static_cast<const void*>(_rDescriptor.m_pFileName.c_str());

            Hash = Base::CRC32(pData, NumberOfBytes);
        }
        else if (_rDescriptor.m_pMaterialName.length() > 0)
        {
            int NumberOfBytes = static_cast<unsigned int>(_rDescriptor.m_pMaterialName.length() * sizeof(char));
            const void* pData = static_cast<const void*>(_rDescriptor.m_pMaterialName.c_str());

            Hash = Base::CRC32(pData, NumberOfBytes);
        }

        assert (Hash != 0);

        if (m_MaterialByHashs.find(Hash) != m_MaterialByHashs.end())
        {
            return *m_MaterialByHashs.at(Hash);
        }

        CInternMaterial* pInternMaterial = 0;

        // -----------------------------------------------------------------------------
        // Create Material
        // -----------------------------------------------------------------------------
        CInternMaterial& rNewMaterial = m_Materials.Allocate();

        pInternMaterial = &rNewMaterial;
        
        // -----------------------------------------------------------------------------
        // Setup material
        // -----------------------------------------------------------------------------
        rNewMaterial.m_Materialname = _rDescriptor.m_pMaterialName;
        rNewMaterial.m_Color        = _rDescriptor.m_AlbedoColor;
        rNewMaterial.m_Roughness    = _rDescriptor.m_Roughness;
        rNewMaterial.m_Reflectance  = _rDescriptor.m_Reflectance;
        rNewMaterial.m_MetalMask    = _rDescriptor.m_MetalMask;
        rNewMaterial.m_Displacement = _rDescriptor.m_Displacement;
        rNewMaterial.m_TilingOffset = _rDescriptor.m_TilingOffset;
        rNewMaterial.m_Hash         = Hash;

        // -----------------------------------------------------------------------------
        // Setup material textures
        // -----------------------------------------------------------------------------
        STextureDescriptor TextureDescriptor;

        TextureDescriptor.m_NumberOfPixelsU  = Dt::STextureDescriptor::s_NumberOfPixelsFromSource;
        TextureDescriptor.m_NumberOfPixelsV  = Dt::STextureDescriptor::s_NumberOfPixelsFromSource;
        TextureDescriptor.m_NumberOfPixelsW  = 1;
        TextureDescriptor.m_Format           = Dt::CTextureBase::R8G8B8_UBYTE;
        TextureDescriptor.m_Semantic         = Dt::CTextureBase::Diffuse;
        TextureDescriptor.m_Binding          = Dt::CTextureBase::ShaderResource;
        TextureDescriptor.m_pPixels          = 0;
        TextureDescriptor.m_pFileName        = 0;
        TextureDescriptor.m_pIdentifier      = 0;

        if (_rDescriptor.m_pColorMap.length() > 0)
        {
            TextureDescriptor.m_pFileName = _rDescriptor.m_pColorMap.c_str();

            rNewMaterial.m_pColorTexture = TextureManager::CreateTexture2D(TextureDescriptor);

            TextureManager::MarkTextureAsDirty(rNewMaterial.m_pColorTexture, CTextureBase::DirtyCreate);
        }

        if (_rDescriptor.m_pNormalMap.length() > 0)
        {
            TextureDescriptor.m_pFileName = _rDescriptor.m_pNormalMap.c_str();

            rNewMaterial.m_pNormalTexture = TextureManager::CreateTexture2D(TextureDescriptor);

            TextureManager::MarkTextureAsDirty(rNewMaterial.m_pNormalTexture, CTextureBase::DirtyCreate);
        }

        if (_rDescriptor.m_pRoughnessMap.length() > 0)
        {
            TextureDescriptor.m_pFileName = _rDescriptor.m_pRoughnessMap.c_str();

            rNewMaterial.m_pRoughnessTexture = TextureManager::CreateTexture2D(TextureDescriptor);
        
            TextureManager::MarkTextureAsDirty(rNewMaterial.m_pRoughnessTexture, CTextureBase::DirtyCreate);
        }

        if (_rDescriptor.m_pMetalMaskMap.length() > 0)
        {
            TextureDescriptor.m_pFileName = _rDescriptor.m_pMetalMaskMap.c_str();

            rNewMaterial.m_pMetalTexture = TextureManager::CreateTexture2D(TextureDescriptor);

            TextureManager::MarkTextureAsDirty(rNewMaterial.m_pMetalTexture, CTextureBase::DirtyCreate);
        }

        if (_rDescriptor.m_pAOMap.length() > 0)
        {
            TextureDescriptor.m_pFileName = _rDescriptor.m_pAOMap.c_str();

            rNewMaterial.m_pAOTexture = TextureManager::CreateTexture2D(TextureDescriptor);

            TextureManager::MarkTextureAsDirty(rNewMaterial.m_pAOTexture, CTextureBase::DirtyCreate);
        }

        if (_rDescriptor.m_pBumpMap.length() > 0)
        {
            TextureDescriptor.m_Format    = CTextureBase::R8_UBYTE;
            TextureDescriptor.m_pFileName = _rDescriptor.m_pBumpMap.c_str();

            rNewMaterial.m_pBumpTexture = TextureManager::CreateTexture2D(TextureDescriptor);

            TextureManager::MarkTextureAsDirty(rNewMaterial.m_pBumpTexture, CTextureBase::DirtyCreate);
        }

        if (Hash != 0)
        {
            m_MaterialByHashs[Hash] = &rNewMaterial;
        }

        return *pInternMaterial;
    }

    // -----------------------------------------------------------------------------

    CMaterial& CDtMaterialManager::GetDefaultMaterial()
    {
        return *m_pDefaultMaterial;
    }

    // -----------------------------------------------------------------------------

    CMaterial& CDtMaterialManager::GetMaterialByHash(unsigned int _Hash)
    {
        if (m_MaterialByHashs.find(_Hash) != m_MaterialByHashs.end())
        {
            return *m_MaterialByHashs.at(_Hash);
        }

        return GetDefaultMaterial();
    }
    
    // -----------------------------------------------------------------------------

    void CDtMaterialManager::FreeMaterial(CInternMaterial& _rMaterial)
    {
        CInternMaterial& rMaterial = static_cast<CInternMaterial&>(_rMaterial);

        m_Materials.Free(&rMaterial);
    }
} // namespace

namespace Dt
{
namespace MaterialManager
{
    void OnStart()
    {
        CDtMaterialManager::GetInstance().OnStart();
    }

    // -----------------------------------------------------------------------------

    void OnExit()
    {
        CDtMaterialManager::GetInstance().OnExit();
    }

    // -----------------------------------------------------------------------------

    CMaterial& CreateMaterial(const SMaterialDescriptor& _rDescriptor)
    {
        return CDtMaterialManager::GetInstance().CreateMaterial(_rDescriptor);
    }

    // -----------------------------------------------------------------------------

    CMaterial& GetDefaultMaterial()
    {
        return CDtMaterialManager::GetInstance().GetDefaultMaterial();
    }

    // -----------------------------------------------------------------------------

    CMaterial& GetMaterialByHash(unsigned int _Hash)
    {
        return CDtMaterialManager::GetInstance().GetMaterialByHash(_Hash);
    }
} // namespace MaterialManager
} // namespace Dt