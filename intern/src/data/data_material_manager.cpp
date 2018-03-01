
#include "data/data_precompiled.h"

#include "base/base_console.h"
#include "base/base_crc.h"
#include "base/base_exception.h"
#include "base/base_include_glm.h"
#include "base/base_memory.h"
#include "base/base_pool.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "core/core_asset_importer.h"

#include "data/data_material.h"
#include "data/data_material_manager.h"

#include <unordered_map>

#include "assimp/Importer.hpp" 
#include "assimp/scene.h" 

#include "tinyxml2.h" 

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

        CMaterial* CreateMaterialFromName(const std::string& _rMaterialname);

        CMaterial* CreateMaterialFromXML(const std::string& _rPathToFile);

        CMaterial* CreateMaterialFromAssimp(const std::string& _rPathToFile, int _MaterialIndex);

        CMaterial* GetMaterialByHash(const CMaterial::BHash _Hash);

        CMaterial* GetDefaultMaterial() const;

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
            float       m_Roughness;
            float       m_Reflectance;
            float       m_MetalMask;
            float       m_Displacement;
            glm::vec3   m_AlbedoColor;
            glm::vec4   m_TilingOffset;
        };


        class CInternMaterial : public CMaterial
        {
        public:

            friend class CDtMaterialManager;
        };

    private:

        typedef Base::CPool<CInternMaterial, 64> CMaterials;
        typedef std::unordered_map<CMaterial::BHash, CInternMaterial*> CMaterialsByHash;

    private:

        static const SMaterialDescriptor s_DefaultDescriptor;
        
    private:

        CMaterials m_Materials;
        CMaterialsByHash m_MaterialsByHash;
        CMaterial* m_pDefaultMaterial;

    private:

        CInternMaterial* CreateMaterialFromDescription(const SMaterialDescriptor& _rDescriptor);
    };
} // namespace

namespace
{
    const CDtMaterialManager::SMaterialDescriptor CDtMaterialManager::s_DefaultDescriptor =
    {
        "STATIC CONST DEFAULT MATERIAL: default.mat", "", "", "", "", "", "",
        1.0f, 0.0f, 0.0f, 0.0f,
        glm::vec3(0.8f),
        glm::vec4(1.0f, 1.0f, 0.0f, 0.0f)
    };
} // namespace 

namespace
{
    CDtMaterialManager::CDtMaterialManager()
        : m_Materials       ( )
        , m_MaterialsByHash ( )
        , m_pDefaultMaterial(0)
    {
        m_pDefaultMaterial = CreateMaterialFromDescription(s_DefaultDescriptor);
    }
    
    // -----------------------------------------------------------------------------
    
    CDtMaterialManager::~CDtMaterialManager()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    CMaterial* CDtMaterialManager::CreateMaterialFromName(const std::string& _rMaterialname)
    {
        // -----------------------------------------------------------------------------
        // Descriptor
        // -----------------------------------------------------------------------------
        SMaterialDescriptor MaterialDescriptor = s_DefaultDescriptor;

        MaterialDescriptor.m_MaterialName = _rMaterialname;

        return CreateMaterialFromDescription(MaterialDescriptor);
    }

        // -----------------------------------------------------------------------------

    CMaterial* CDtMaterialManager::CreateMaterialFromXML(const std::string& _rPathToFile)
    {
        // -----------------------------------------------------------------------------
        // Descriptor
        // -----------------------------------------------------------------------------
        SMaterialDescriptor MaterialDescriptor = s_DefaultDescriptor;

        // -----------------------------------------------------------------------------
        // Load data
        // -----------------------------------------------------------------------------
        auto Importer = Core::AssetImporter::AllocateTinyXMLImporter(_rPathToFile);

        if (Importer == nullptr) return m_pDefaultMaterial;

        tinyxml2::XMLDocument* pMaterialFile = static_cast<tinyxml2::XMLDocument*>(Core::AssetImporter::GetNativeAccessFromImporter(Importer));

        if (pMaterialFile == nullptr) return m_pDefaultMaterial;

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
        // Values
        // -----------------------------------------------------------------------------
        MaterialDescriptor.m_MaterialName = std::string(pElementDefinition->Attribute("Name")) + ": " + _rPathToFile;

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
        if (pElementColor != nullptr && pElementColor->Attribute("Map")) MaterialDescriptor.m_ColorTexture = pElementColor->Attribute("Map");
        if (pElementNormal != nullptr && pElementNormal->Attribute("Map")) MaterialDescriptor.m_NormalTexture = pElementNormal->Attribute("Map");
        if (pElementRoughness != nullptr && pElementRoughness->Attribute("Map")) MaterialDescriptor.m_RoughnessTexture = pElementRoughness->Attribute("Map");
        if (pElementMetallic != nullptr && pElementMetallic->Attribute("Map")) MaterialDescriptor.m_MetalTexture = pElementMetallic->Attribute("Map");
        if (pElementBump != nullptr && pElementBump->Attribute("Map")) MaterialDescriptor.m_BumpTexture = pElementBump->Attribute("Map");
        if (pElementAO != nullptr && pElementAO->Attribute("Map")) MaterialDescriptor.m_AmbientOcclusionTexture = pElementAO->Attribute("Map");

        return CreateMaterialFromDescription(MaterialDescriptor);
    }

    // -----------------------------------------------------------------------------

    CMaterial* CDtMaterialManager::CreateMaterialFromAssimp(const std::string& _rPathToFile, int _MaterialIndex)
    {
        // -----------------------------------------------------------------------------
        // Descriptor
        // -----------------------------------------------------------------------------
        SMaterialDescriptor MaterialDescriptor = s_DefaultDescriptor;

        // -----------------------------------------------------------------------------
        // Importer
        // -----------------------------------------------------------------------------
        auto Importer = Core::AssetImporter::AllocateAssimpImporter(_rPathToFile, Core::AssetGenerator::SGeneratorFlag::Nothing);

        if (Importer == nullptr) return m_pDefaultMaterial;

        const Assimp::Importer* pImporter = static_cast<const Assimp::Importer*>(Core::AssetImporter::GetNativeAccessFromImporter(Importer));

        if (pImporter == nullptr) return m_pDefaultMaterial;

        const aiScene* pScene = pImporter->GetScene();

        if (pScene == nullptr) return m_pDefaultMaterial;

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

            // -----------------------------------------------------------------------------
            // Check diffuse material if a *.mat file is set
            // -----------------------------------------------------------------------------
            if (MaterialDescriptor.m_ColorTexture.length() > 0)
            {
                if (MaterialDescriptor.m_ColorTexture.find(".mat") != std::string::npos)
                {
                    std::string PathToMat = _rPathToFile.substr(0, _rPathToFile.find_last_of('/')) + "/" + MaterialDescriptor.m_ColorTexture;

                    return CreateMaterialFromXML(PathToMat);
                }
            }
        }

        return CreateMaterialFromDescription(MaterialDescriptor);
    }

    // -----------------------------------------------------------------------------

    CMaterial* CDtMaterialManager::GetMaterialByHash(const CMaterial::BHash _Hash)
    {
        if (m_MaterialsByHash.find(_Hash) == m_MaterialsByHash.end()) return m_pDefaultMaterial;
        
        return m_MaterialsByHash.at(_Hash);
    }

    // -----------------------------------------------------------------------------

    CMaterial* CDtMaterialManager::GetDefaultMaterial() const
    {
        return m_pDefaultMaterial;
    }

    // -----------------------------------------------------------------------------

    CDtMaterialManager::CInternMaterial* CDtMaterialManager::CreateMaterialFromDescription(const SMaterialDescriptor& _rDescriptor)
    {
        // -----------------------------------------------------------------------------
        // Hash
        // -----------------------------------------------------------------------------
        Base::BHash Hash = Base::CRC32(_rDescriptor.m_MaterialName.c_str(), static_cast<unsigned int>(_rDescriptor.m_MaterialName.length()));

        if (m_MaterialsByHash.find(Hash) != m_MaterialsByHash.end())
        {
            return m_MaterialsByHash.at(Hash);
        }

        // -----------------------------------------------------------------------------
        // Create material
        // -----------------------------------------------------------------------------
        auto& rComponent = m_Materials.Allocate();

        rComponent.m_Materialname            = _rDescriptor.m_MaterialName;
        rComponent.m_ColorTexture            = _rDescriptor.m_ColorTexture;
        rComponent.m_NormalTexture           = _rDescriptor.m_NormalTexture;
        rComponent.m_RoughnessTexture        = _rDescriptor.m_RoughnessTexture;
        rComponent.m_MetalTexture            = _rDescriptor.m_MetalTexture;
        rComponent.m_AmbientOcclusionTexture = _rDescriptor.m_AmbientOcclusionTexture;
        rComponent.m_BumpTexture             = _rDescriptor.m_BumpTexture;
        rComponent.m_Roughness               = _rDescriptor.m_Roughness;
        rComponent.m_Reflectance             = _rDescriptor.m_Reflectance;
        rComponent.m_MetalMask               = _rDescriptor.m_MetalMask;
        rComponent.m_Displacement            = _rDescriptor.m_Displacement;
        rComponent.m_Color                   = _rDescriptor.m_AlbedoColor;
        rComponent.m_TilingOffset            = _rDescriptor.m_TilingOffset;

        rComponent.m_Hash = Hash;

        // -----------------------------------------------------------------------------
        // Add component to hash map
        // -----------------------------------------------------------------------------
        m_MaterialsByHash[Hash] = &rComponent;

        return &rComponent;
    }
} // namespace

namespace Dt
{
namespace MaterialManager
{
    CMaterial* CreateMaterialFromName(const std::string& _rMaterialname)
    {
        return CDtMaterialManager::GetInstance().CreateMaterialFromName(_rMaterialname);
    }

    // -----------------------------------------------------------------------------

    CMaterial* CreateMaterialFromXML(const std::string& _rPathToFile)
    {
        return CDtMaterialManager::GetInstance().CreateMaterialFromXML(_rPathToFile);
    }

    // -----------------------------------------------------------------------------

    CMaterial* CreateMaterialFromAssimp(const std::string& _rPathToFile, int _MaterialIndex)
    {
        return CDtMaterialManager::GetInstance().CreateMaterialFromAssimp(_rPathToFile, _MaterialIndex);
    }

    // -----------------------------------------------------------------------------

    CMaterial* GetMaterialByHash(const CMaterial::BHash _Hash)
    {
        return CDtMaterialManager::GetInstance().GetMaterialByHash(_Hash);
    }

    // -----------------------------------------------------------------------------

    CMaterial* GetDefaultMaterial()
    {
        return CDtMaterialManager::GetInstance().GetDefaultMaterial();
    }
} // namespace MaterialManager
} // namespace Dt