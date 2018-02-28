
#include "data/data_precompiled.h"

#include "base/base_console.h"
#include "base/base_crc.h"
#include "base/base_exception.h"
#include "base/base_include_glm.h"
#include "base/base_memory.h"
#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "core/core_asset_importer.h"
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

        CMaterialComponent* m_pDefaultMaterial;

    private:

        void FillMaterialFromXML(CMaterialComponent* _pMaterial, const const std::string& _rFilename);
    };
} // namespace

namespace
{
    CDtMaterialManager::CDtMaterialManager()
        : m_pDefaultMaterial(0)
    {
        m_pDefaultMaterial = Dt::CComponentManager::GetInstance().Allocate<CMaterialComponent>();
    }
    
    // -----------------------------------------------------------------------------
    
    CDtMaterialManager::~CDtMaterialManager()
    {
    }
    
    // -----------------------------------------------------------------------------
    
    CMaterialComponent* CDtMaterialManager::CreateMaterialFromFile(const std::string& _rFilename)
    {
        if (_rFilename.find(".mat") == std::string::npos)
        {
            BASE_CONSOLE_ERROR("Only internal materials are accepted.");

            return nullptr;
        }

        // -----------------------------------------------------------------------------
        // Create hash value
        // -----------------------------------------------------------------------------
        auto pComponent = Dt::CComponentManager::GetInstance().Allocate<CMaterialComponent>();

        FillMaterialFromXML(pComponent, _rFilename);

        Dt::CComponentManager::GetInstance().MarkComponentAsDirty(pComponent, CMaterialComponent::DirtyCreate);

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

        // -----------------------------------------------------------------------------
        // Release importer
        // -----------------------------------------------------------------------------
        Core::AssetImporter::ReleaseImporter(Importer);
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