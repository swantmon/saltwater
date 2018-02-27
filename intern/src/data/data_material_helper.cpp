
#include "data/data_precompiled.h"

#include "base/base_console.h"
#include "base/base_crc.h"
#include "base/base_exception.h"
#include "base/base_include_glm.h"
#include "base/base_memory.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "core/core_asset_manager.h"

#include "data/data_component.h"
#include "data/data_material_component.h"
#include "data/data_material_helper.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"

#include "tinyxml2.h"

#include <unordered_map>

using namespace Dt;
using namespace Dt::MaterialHelper;

namespace
{
    class CDtMaterialManager : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CDtMaterialManager)
        
    public:
        
        CDtMaterialManager();
        ~CDtMaterialManager();
        
    public:

        CMaterialComponent* CreateMaterialFromFile(const std::string& _rFilename);

        const CMaterialComponent* GetDefaultMaterial() const;

    private:

        typedef std::unordered_map<Base::BHash, CMaterialComponent*> CMaterialByHashs;
        
    private:
        
        CMaterialByHashs m_MaterialByHashs;

        CMaterialComponent* m_pDefaultMaterial;

    private:

        void FillMaterialFromXML(CMaterialComponent* _pMaterial, const const std::string& _rFilename);

        void FillMaterialFromAssimp(CMaterialComponent* _pMaterial, const const std::string& _rFilename);
    };
} // namespace

namespace
{
    CDtMaterialManager::CDtMaterialManager()
        : m_MaterialByHashs ()
        , m_pDefaultMaterial(0)
    {
        m_pDefaultMaterial = Dt::CComponentManager::GetInstance().Allocate<CMaterialComponent>();
    }
    
    // -----------------------------------------------------------------------------
    
    CDtMaterialManager::~CDtMaterialManager()
    {
        m_MaterialByHashs.clear();
    }
    
    // -----------------------------------------------------------------------------
    
    CMaterialComponent* CDtMaterialManager::CreateMaterialFromFile(const std::string& _rFilename)
    {
        if (_rFilename.length() == 0)
        {
            BASE_THROWM("No material filename was given to load material.")
        }

        // -----------------------------------------------------------------------------
        // Create hash value
        // -----------------------------------------------------------------------------
        int NumberOfBytes = static_cast<unsigned int>(_rFilename.length() * sizeof(char));
        const void* pData = static_cast<const void*>(_rFilename.c_str());

        Base::BHash Hash = Base::CRC32(pData, NumberOfBytes);

        if (m_MaterialByHashs.find(Hash) != m_MaterialByHashs.end())
        {
            return m_MaterialByHashs.at(Hash);
        }

        auto pComponent = Dt::CComponentManager::GetInstance().Allocate<CMaterialComponent>();

        if (_rFilename.find(".mat") != std::string::npos)
        {
            FillMaterialFromXML(pComponent, _rFilename);
        }
        else
        {
            FillMaterialFromAssimp(pComponent, _rFilename);
        }

        Dt::CComponentManager::GetInstance().MarkComponentAsDirty(pComponent, CMaterialComponent::DirtyCreate);

        m_MaterialByHashs[Hash] = pComponent;

        return pComponent;
    }

    // -----------------------------------------------------------------------------

    const CMaterialComponent* CDtMaterialManager::GetDefaultMaterial() const
    {
        return m_pDefaultMaterial;
    }

    // -----------------------------------------------------------------------------

    void CDtMaterialManager::FillMaterialFromXML(CMaterialComponent* _pMaterial, const std::string& _rFilename)
    {
        tinyxml2::XMLDocument MaterialFile;

        // -----------------------------------------------------------------------------
        // Build path to texture in file system
        // -----------------------------------------------------------------------------
        std::string PathToMaterial = Core::AssetManager::GetPathToAssets() + "/" + _rFilename;

        // -----------------------------------------------------------------------------
        // Load material file
        // -----------------------------------------------------------------------------
        int Error = MaterialFile.LoadFile(PathToMaterial.c_str());

        if (Error != tinyxml2::XML_SUCCESS)
        {
            BASE_CONSOLE_ERRORV("Loading material file '%s' failed.", PathToMaterial.c_str());

            return;
        }

        tinyxml2::XMLElement* pElementDefinition  = MaterialFile.FirstChildElement("MaterialDefinition");
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
        _pMaterial->SetMaterialname(pElementDefinition->Attribute("Name"));

        if (pElementColor != 0)
        {
            float ColorR = pElementColor->FloatAttribute("R");
            float ColorG = pElementColor->FloatAttribute("G");
            float ColorB = pElementColor->FloatAttribute("B");

            _pMaterial->SetColor(glm::vec3(ColorR, ColorG, ColorB));
        }

        if (pElementRoughness) _pMaterial->SetRoughness(pElementRoughness->FloatAttribute("V"));
        if (pElementReflectance) _pMaterial->SetReflectance(pElementReflectance->FloatAttribute("V"));
        if (pElementMetallic) _pMaterial->SetMetalness(pElementMetallic->FloatAttribute("V"));
        if (pElementBump) _pMaterial->SetDisplacement(pElementBump->FloatAttribute("V"));

        if (pElementTiling)
        {
            _pMaterial->SetTiling(glm::vec2(pElementTiling->FloatAttribute("X"), pElementTiling->FloatAttribute("Y")));
        }

        if (pElementOffset)
        {
            _pMaterial->SetOffset(glm::vec2(pElementOffset->FloatAttribute("X"), pElementOffset->FloatAttribute("Y")));
        }

        // -----------------------------------------------------------------------------
        // Textures
        // -----------------------------------------------------------------------------
        if (pElementColor != nullptr && pElementColor->Attribute("Map")) _pMaterial->SetColorTexture(pElementColor->Attribute("Map"));
        if (pElementNormal != nullptr && pElementNormal->Attribute("Map")) _pMaterial->SetNormalTexture(pElementNormal->Attribute("Map"));
        if (pElementRoughness != nullptr && pElementRoughness->Attribute("Map")) _pMaterial->SetRoughnessTexture(pElementRoughness->Attribute("Map"));
        if (pElementMetallic != nullptr && pElementMetallic->Attribute("Map")) _pMaterial->SetMetalTexture(pElementMetallic->Attribute("Map"));
        if (pElementBump != nullptr && pElementBump->Attribute("Map")) _pMaterial->SetBumpTexture(pElementBump->Attribute("Map"));
        if (pElementAO != nullptr && pElementAO->Attribute("Map")) _pMaterial->SetAmbientOcclusionTexture(pElementAO->Attribute("Map"));
    }

    // -----------------------------------------------------------------------------

    void CDtMaterialManager::FillMaterialFromAssimp(CMaterialComponent* _pMaterial, const const std::string& _rFilename)
    {
        Assimp::Importer Importer;

        std::string PathToMaterial = Core::AssetManager::GetPathToAssets() + "/" + _rFilename;

        const aiScene* pScene = Importer.ReadFile(PathToMaterial.c_str(), 0);

        if (!pScene)
        {
            BASE_CONSOLE_ERRORV("Loading material file '%s' failed. Code: %s.", PathToMaterial.c_str(), Importer.GetErrorString());

            return;
        }

        // -----------------------------------------------------------------------------
        // Only single materials are currently supported!
        // Question: Do wee need multiple materials?
        // -----------------------------------------------------------------------------
        unsigned int NumberOfMaterials = pScene->mNumMaterials;

        assert(NumberOfMaterials == 1);

        aiMaterial* pAssimpMaterial = pScene->mMaterials[0];

        // -----------------------------------------------------------------------------
        // Fill data
        // -----------------------------------------------------------------------------
        assert(_pMaterial);

        // -----------------------------------------------------------------------------
        // Values and textures
        // -----------------------------------------------------------------------------
        aiString  NativeString;
        aiColor4D DiffuseColor;

        if (pAssimpMaterial->Get(AI_MATKEY_NAME, NativeString) == AI_SUCCESS)
        {
            _pMaterial->SetMaterialname(NativeString.data);
        }

        if (pAssimpMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, DiffuseColor) == AI_SUCCESS)
        {
            _pMaterial->SetColor(glm::vec3(DiffuseColor.r, DiffuseColor.g, DiffuseColor.b));
        }

        if (pAssimpMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &NativeString) == AI_SUCCESS)
        {
            _pMaterial->SetColorTexture(NativeString.data);
        }

        if (pAssimpMaterial->GetTexture(aiTextureType_HEIGHT, 0, &NativeString) == AI_SUCCESS)
        {
            _pMaterial->SetNormalTexture(NativeString.data);
        }
    }
} // namespace

namespace Dt
{
namespace MaterialHelper
{
    CMaterialComponent* CreateMaterialFromFile(const std::string& _rFilename)
    {
        return CDtMaterialManager::GetInstance().CreateMaterialFromFile(_rFilename);
    }

    // -----------------------------------------------------------------------------

    const CMaterialComponent* GetDefaultMaterial()
    {
        return CDtMaterialManager::GetInstance().GetDefaultMaterial();
    }
} // namespace MaterialHelper
} // namespace Dt