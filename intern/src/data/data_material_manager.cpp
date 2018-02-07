
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
#include "data/data_lod.h"
#include "data/data_map.h"
#include "data/data_material.h"
#include "data/data_material_manager.h"
#include "data/data_surface.h"
#include "data/data_texture_2d.h"
#include "data/data_texture_manager.h"

#include "tinyxml2.h"

#include <functional>
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

        void MarkMaterialAsDirty(CMaterial& _rMaterial, unsigned int _DirtyFlags);

        void RegisterDirtyMaterialHandler(CMaterialDelegate _NewDelegate);
        
    private:
        
        class CInternMaterial : public CMaterial
        {
        private:
            
            friend class CDtMaterialManager;
        };
        
    private:
        
        typedef Base::CPool<CInternMaterial, 1024> CMaterials;

        typedef std::vector<CMaterialDelegate> CMaterialDelegates;

        typedef std::unordered_map<unsigned int, CInternMaterial*> CMaterialByHashs;
        
    private:
        
        CMaterials         m_Materials;
        CMaterialDelegates m_MaterialDelegates;
        CMaterialByHashs   m_MaterialByHashs;

        CInternMaterial* m_pDefaultMaterial;

    private:

        void FreeMaterial(CInternMaterial& _rMaterial);
    };
} // namespace

namespace
{
    CDtMaterialManager::CDtMaterialManager()
        : m_Materials        ()
        , m_MaterialDelegates()
        , m_MaterialByHashs  ()
        , m_pDefaultMaterial (0)
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
        MaterialDescriptor.m_AlbedoColor     = glm::vec3(1.0f);
        MaterialDescriptor.m_TilingOffset    = glm::vec4(0.0f);
        MaterialDescriptor.m_pFileName       = 0;

        m_pDefaultMaterial = &static_cast<CInternMaterial&>(CreateMaterial(MaterialDescriptor));
    }

    // -----------------------------------------------------------------------------

    void CDtMaterialManager::OnExit()
    {
        m_Materials.Free(m_pDefaultMaterial);

        m_pDefaultMaterial = 0;

        // -----------------------------------------------------------------------------

        m_Materials.Clear();

        m_MaterialDelegates.clear();

        m_MaterialByHashs.clear();
    }
    
    // -----------------------------------------------------------------------------
    
    CMaterial& CDtMaterialManager::CreateMaterial(const SMaterialDescriptor& _rDescriptor)
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
        glm::vec3          AlbedoColor;
        glm::vec4          TilingOffset;
        int                   NumberOfBytes;
        unsigned int          Hash;
        tinyxml2::XMLDocument MaterialFile;

        Hash = 0;

        // -----------------------------------------------------------------------------
        // Create hash value
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

        assert (Hash != 0);

        if (m_MaterialByHashs.find(Hash) != m_MaterialByHashs.end())
        {
            return *m_MaterialByHashs.at(Hash);
        }

        // -----------------------------------------------------------------------------
        // Setup default values
        // -----------------------------------------------------------------------------
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

        if (_rDescriptor.m_pFileName != nullptr)
        {
            // -----------------------------------------------------------------------------
            // Build path to texture in file system
            // -----------------------------------------------------------------------------
            std::string PathToMaterial = Core::AssetManager::GetPathToAssets() + "/" + _rDescriptor.m_pFileName;
        
            // -----------------------------------------------------------------------------
            // Load material file
            // -----------------------------------------------------------------------------
            int Error = MaterialFile.LoadFile(PathToMaterial.c_str());

            if (Error != tinyxml2::XML_SUCCESS)
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

            AlbedoColor = glm::vec3(ColorR, ColorG, ColorB);

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

                Displacement = pBump->FloatAttribute("V");
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

        CInternMaterial* pInternMaterial = 0;

        try
        {
            // -----------------------------------------------------------------------------
            // Create Material
            // -----------------------------------------------------------------------------
            CInternMaterial& rNewMaterial = m_Materials.Allocate();

            pInternMaterial = &rNewMaterial;
        
            // -----------------------------------------------------------------------------
            // Setup material
            // -----------------------------------------------------------------------------
            rNewMaterial.m_Materialname = pMaterialName;
            rNewMaterial.m_Color        = AlbedoColor;
            rNewMaterial.m_Roughness    = Roughness;
            rNewMaterial.m_Reflectance  = Reflectance;
            rNewMaterial.m_MetalMask    = MetalMask;
            rNewMaterial.m_Displacement = Displacement;
            rNewMaterial.m_TilingOffset = TilingOffset;
            rNewMaterial.m_Hash         = Hash;

            if (_rDescriptor.m_pFileName != 0) rNewMaterial.m_FileName = _rDescriptor.m_pFileName;

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

            if (pColorMap)
            {
                TextureDescriptor.m_pFileName = pColorMap;

                rNewMaterial.m_pColorTexture = TextureManager::CreateTexture2D(TextureDescriptor);

                TextureManager::MarkTextureAsDirty(rNewMaterial.m_pColorTexture, CTextureBase::DirtyCreate);
            }

            if (pNormalMap)
            {
                TextureDescriptor.m_pFileName = pNormalMap;

                rNewMaterial.m_pNormalTexture = TextureManager::CreateTexture2D(TextureDescriptor);

                TextureManager::MarkTextureAsDirty(rNewMaterial.m_pNormalTexture, CTextureBase::DirtyCreate);
            }

            if (pRoughnessMap)
            {
                TextureDescriptor.m_pFileName = pRoughnessMap;

                rNewMaterial.m_pRoughnessTexture = TextureManager::CreateTexture2D(TextureDescriptor);
        
                TextureManager::MarkTextureAsDirty(rNewMaterial.m_pRoughnessTexture, CTextureBase::DirtyCreate);
            }

            if (pMetalMaskMap)
            {
                TextureDescriptor.m_pFileName = pMetalMaskMap;

                rNewMaterial.m_pMetalTexture = TextureManager::CreateTexture2D(TextureDescriptor);

                TextureManager::MarkTextureAsDirty(rNewMaterial.m_pMetalTexture, CTextureBase::DirtyCreate);
            }

            if (pAOMap)
            {
                TextureDescriptor.m_pFileName = pAOMap;

                rNewMaterial.m_pAOTexture = TextureManager::CreateTexture2D(TextureDescriptor);

                TextureManager::MarkTextureAsDirty(rNewMaterial.m_pAOTexture, CTextureBase::DirtyCreate);
            }

            if (pBumpMap)
            {
                TextureDescriptor.m_Format    = CTextureBase::R8_UBYTE;
                TextureDescriptor.m_pFileName = pBumpMap;

                rNewMaterial.m_pBumpTexture = TextureManager::CreateTexture2D(TextureDescriptor);

                TextureManager::MarkTextureAsDirty(rNewMaterial.m_pBumpTexture, CTextureBase::DirtyCreate);
            }

            if (Hash != 0)
            {
                m_MaterialByHashs[Hash] = &rNewMaterial;
            }
        }
        catch (...)
        {
            BASE_CONSOLE_STREAMERROR("Failed create an material.");

            return *m_pDefaultMaterial;
        }

        if (_rDescriptor.m_pFileName != nullptr)
        {
            MaterialFile.Clear();
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

    void CDtMaterialManager::MarkMaterialAsDirty(CMaterial& _rMaterial, unsigned int _DirtyFlags)
    {
        CInternMaterial& rMaterial = static_cast<CInternMaterial&>(_rMaterial);

        // -----------------------------------------------------------------------------
        // Flag
        // -----------------------------------------------------------------------------
        rMaterial.m_DirtyFlags = _DirtyFlags;

        // -----------------------------------------------------------------------------
        // Dirty time
        // -----------------------------------------------------------------------------
        Base::U64 FrameTime = Core::Time::GetNumberOfFrame();

        rMaterial.m_DirtyTime = FrameTime;

        // -----------------------------------------------------------------------------
        // Send new dirty entity to all handler
        // -----------------------------------------------------------------------------
        CMaterialDelegates::iterator CurrentDirtyDelegate = m_MaterialDelegates.begin();
        CMaterialDelegates::iterator EndOfDirtyDelegates  = m_MaterialDelegates.end();

        for (; CurrentDirtyDelegate != EndOfDirtyDelegates; ++CurrentDirtyDelegate)
        {
            (*CurrentDirtyDelegate)(&rMaterial);
        }

        // -----------------------------------------------------------------------------
        // Handle dirty flag
        // -----------------------------------------------------------------------------
        if ((_DirtyFlags & Dt::CMaterial::DirtyDestroy) != 0)
        {
            FreeMaterial(rMaterial);
        }
    }

    // -----------------------------------------------------------------------------

    void CDtMaterialManager::RegisterDirtyMaterialHandler(CMaterialDelegate _NewDelegate)
    {
        m_MaterialDelegates.push_back(_NewDelegate);
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

    // -----------------------------------------------------------------------------

    void MarkMaterialAsDirty(CMaterial& _rMaterial, unsigned int _DirtyFlags)
    {
        CDtMaterialManager::GetInstance().MarkMaterialAsDirty(_rMaterial, _DirtyFlags);
    }

    // -----------------------------------------------------------------------------

    void RegisterDirtyMaterialHandler(CMaterialDelegate _NewDelegate)
    {
        CDtMaterialManager::GetInstance().RegisterDirtyMaterialHandler(_NewDelegate);
    }
} // namespace MaterialManager
} // namespace Dt