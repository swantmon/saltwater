
#include "data/data_precompiled.h"

#include "base/base_aabb3.h"
#include "base/base_console.h"
#include "base/base_crc.h"
#include "base/base_exception.h"
#include "base/base_math_constants.h"
#include "base/base_math_operations.h"
#include "base/base_memory.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"
#include "base/base_vector2.h"
#include "base/base_vector3.h"
#include "base/base_pool.h"

#include "data/data_entity.h"
#include "data/data_lod.h"
#include "data/data_map.h"
#include "data/data_material.h"
#include "data/data_material_manager.h"
#include "data/data_surface.h"
#include "data/data_texture_2d.h"
#include "data/data_texture_manager.h"

#include "tinyxml2.h"

#include <unordered_map>
#include <functional>

using namespace Dt;
using namespace Dt::MaterialManager;

namespace 
{
	std::string g_PathToAssets = "../assets/";
} // namespace 

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

        void Clear();

        CMaterial& CreateEmptyMaterial();
        CMaterial& CreateMaterial(const SMaterialFileDescriptor& _rDescriptor);
        
        void FreeMaterial(CMaterial& _rMaterial);
        
    private:
        
        class CInternMaterial : public CMaterial
        {
        private:
            
            friend class CDtMaterialManager;
        };
        
    private:
        
        typedef Base::CPool<CInternMaterial, 1024> CMaterials;
        
    private:
        
        CMaterials m_Materials;
    };
} // namespace

namespace
{
    CDtMaterialManager::CDtMaterialManager()
        : m_Materials()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    CDtMaterialManager::~CDtMaterialManager()
    {
    }

    // -----------------------------------------------------------------------------

    void CDtMaterialManager::OnStart()
    {
    }

    // -----------------------------------------------------------------------------

    void CDtMaterialManager::OnExit()
    {
        Clear();
    }

    // -----------------------------------------------------------------------------

    void CDtMaterialManager::Clear()
    {
        m_Materials.Clear();
    }

    // -----------------------------------------------------------------------------

    CMaterial& CDtMaterialManager::CreateEmptyMaterial()
    {
        // -----------------------------------------------------------------------------
        // Create Material
        // -----------------------------------------------------------------------------
        CInternMaterial& rNewMaterial = m_Materials.Allocate();

        rNewMaterial.m_Materialname = "";

        return rNewMaterial;
    }
    
    // -----------------------------------------------------------------------------
    
    CMaterial& CDtMaterialManager::CreateMaterial(const SMaterialFileDescriptor& _rDescriptor)
    {
        if (_rDescriptor.m_pFileName == 0)
        {
            BASE_THROWM("Can't create material because of invalid informations.");
        }
        
        // -----------------------------------------------------------------------------
        // Create Material
        // -----------------------------------------------------------------------------
        CInternMaterial& rNewMaterial = m_Materials.Allocate();

        rNewMaterial.m_Materialname = _rDescriptor.m_pFileName;
        
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
        const char* pMaterialName = pMaterialDefinition->Attribute("Name");
        
        assert(pMaterialName != 0);
        
        // -----------------------------------------------------------------------------
        // Color
        // -----------------------------------------------------------------------------
        tinyxml2::XMLElement* pMaterialColor = pMaterialDefinition->FirstChildElement("Color");
        
        assert(pMaterialColor != 0);
        
        float ColorR = pMaterialColor->FloatAttribute("R");
        float ColorG = pMaterialColor->FloatAttribute("G");
        float ColorB = pMaterialColor->FloatAttribute("B");
        
        const char* pColorMap = pMaterialColor->Attribute("Map");
        
        // -----------------------------------------------------------------------------
        // Normal
        // -----------------------------------------------------------------------------
        tinyxml2::XMLElement* pMaterialNormal = pMaterialDefinition->FirstChildElement("Normal");
        
        assert(pMaterialNormal != 0);
        
        const char* pNormalMap = pMaterialNormal->Attribute("Map");
        
        // -----------------------------------------------------------------------------
        // Roughness
        // -----------------------------------------------------------------------------
        tinyxml2::XMLElement* pMaterialRoughness = pMaterialDefinition->FirstChildElement("Roughness");
        
        assert(pMaterialRoughness != 0);
        
        float Roughness = pMaterialRoughness->FloatAttribute("V");
        
        const char* pRoughnessMap = pMaterialRoughness->Attribute("Map");
        
        // -----------------------------------------------------------------------------
        // Reflectance
        // -----------------------------------------------------------------------------
        tinyxml2::XMLElement* pMaterialReflectance = pMaterialDefinition->FirstChildElement("Reflectance");
        
        assert(pMaterialReflectance != 0);
        
        float Reflectance = pMaterialReflectance->FloatAttribute("V");
        
        const char* pReflectanceMap = pMaterialReflectance->Attribute("Map");
        
        // -----------------------------------------------------------------------------
        // Metallic
        // -----------------------------------------------------------------------------
        tinyxml2::XMLElement* pMaterialMetallic = pMaterialDefinition->FirstChildElement("Metallic");
        
        assert(pMaterialMetallic != 0);
        
        float MetalMask = pMaterialMetallic->FloatAttribute("V");
        
        const char* pMetalMaskMap = pMaterialMetallic->Attribute("Map");
        
        // -----------------------------------------------------------------------------
        // Ambient Occlusion
        // -----------------------------------------------------------------------------
        tinyxml2::XMLElement* pAO = pMaterialDefinition->FirstChildElement("AO");
        
        const char* pAOMap = 0;
        
        if (pAO != 0)
        {
            pAOMap = pAO->Attribute("Map");
        }
        
        // -----------------------------------------------------------------------------
        // Bump
        // -----------------------------------------------------------------------------
        tinyxml2::XMLElement* pBump = pMaterialDefinition->FirstChildElement("Bump");
        
        const char* pBumpMap = 0;
        
        if (pBump != 0)
        {
            pBumpMap = pBump->Attribute("Map");
        }
        
        // -----------------------------------------------------------------------------
        // Tiling & offset
        // -----------------------------------------------------------------------------
        tinyxml2::XMLElement* pTiling = pMaterialDefinition->FirstChildElement("Tiling");
        tinyxml2::XMLElement* pOffset = pMaterialDefinition->FirstChildElement("Offset");
        
        Base::Float4 TilingOffset(1.0f, 1.0f, 0.0f, 0.0f);
        
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
        
        // -----------------------------------------------------------------------------
        // Setup material
        // -----------------------------------------------------------------------------
        rNewMaterial.m_Materialname = pMaterialName;
        rNewMaterial.m_Color        = Base::Float3(ColorR, ColorG, ColorB);
        rNewMaterial.m_Roughness    = Roughness;
        rNewMaterial.m_Reflectance  = Reflectance;
        rNewMaterial.m_MetalMask    = MetalMask;
        rNewMaterial.m_TilingOffset = TilingOffset;

        // -----------------------------------------------------------------------------
        // Setup material textures
        // -----------------------------------------------------------------------------
        STextureDescriptor TextureDescriptor;

        TextureDescriptor.m_NumberOfPixelsU  = Dt::STextureDescriptor::s_NumberOfPixelsFromSource;
        TextureDescriptor.m_NumberOfPixelsV  = Dt::STextureDescriptor::s_NumberOfPixelsFromSource;
        TextureDescriptor.m_NumberOfPixelsW  = 1;
        TextureDescriptor.m_Format           = Dt::CTextureBase::R8G8B8_UBYTE;
        TextureDescriptor.m_Semantic         = Dt::CTextureBase::Diffuse;
        TextureDescriptor.m_pPixels          = 0;
        TextureDescriptor.m_pFileName        = 0;
        TextureDescriptor.m_pIdentifier      = 0;

        if (pColorMap)
        {
            TextureDescriptor.m_pFileName = pColorMap;

            rNewMaterial.m_pColorTexture = TextureManager::CreateTexture2D(TextureDescriptor);
        }

        if (pNormalMap)
        {
            TextureDescriptor.m_pFileName = pNormalMap;

            rNewMaterial.m_pNormalTexture = TextureManager::CreateTexture2D(TextureDescriptor);
        }

        if (pRoughnessMap)
        {
            TextureDescriptor.m_pFileName = pRoughnessMap;

            rNewMaterial.m_pRoughnessTexture = TextureManager::CreateTexture2D(TextureDescriptor);
        }

        if (pReflectanceMap)
        {
            TextureDescriptor.m_pFileName = pReflectanceMap;

            rNewMaterial.m_pReflectanceMap = TextureManager::CreateTexture2D(TextureDescriptor);
        }

        if (pMetalMaskMap)
        {
            TextureDescriptor.m_pFileName = pMetalMaskMap;

            rNewMaterial.m_pMetalTexture = TextureManager::CreateTexture2D(TextureDescriptor);
        }

        if (pAOMap)
        {
            TextureDescriptor.m_pFileName = pAOMap;

            rNewMaterial.m_pAOTexture = TextureManager::CreateTexture2D(TextureDescriptor);
        }

        if (pBumpMap)
        {
            TextureDescriptor.m_Format    = CTextureBase::R8_UBYTE;
            TextureDescriptor.m_pFileName = pBumpMap;

            rNewMaterial.m_pBumpTexture = TextureManager::CreateTexture2D(TextureDescriptor);
        }

        MaterialFile.Clear();
        
        return rNewMaterial;
    }
    
    // -----------------------------------------------------------------------------
    
    void CDtMaterialManager::FreeMaterial(CMaterial& _rMaterial)
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

    void Clear()
    {
        CDtMaterialManager::GetInstance().Clear();
    }

    // -----------------------------------------------------------------------------

    CMaterial& CreateEmptyMaterial()
    {
        return CDtMaterialManager::GetInstance().CreateEmptyMaterial();
    }

    // -----------------------------------------------------------------------------

    CMaterial& CreateMaterial(const SMaterialFileDescriptor& _rDescriptor)
    {
        return CDtMaterialManager::GetInstance().CreateMaterial(_rDescriptor);
    }
    
    // -----------------------------------------------------------------------------
    
    void FreeMaterial(CMaterial& _rMaterial)
    {
        CDtMaterialManager::GetInstance().FreeMaterial(_rMaterial);
    }
} // namespace MaterialManager
} // namespace Dt