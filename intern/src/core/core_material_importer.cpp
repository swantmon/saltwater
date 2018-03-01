
#include "core/core_precompiled.h"

#include "base/base_singleton.h"
#include "base/base_uncopyable.h"

#include "core/core_asset_importer.h"
#include "core/core_material_importer.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"

#include "tinyxml2.h"

using namespace Core;
using namespace Core::MaterialImporter;

namespace 
{
    class CCoreMaterialImporter : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CCoreMaterialImporter)

    public:

        SMaterialDescriptor CreateDescriptionFromXML(const std::string& _rPathToFile) const;

        SMaterialDescriptor CreateDescriptionFromAssimpFile(const std::string& _rPathToFile, int _MaterialIndex) const;

    private:

        CCoreMaterialImporter();
        ~CCoreMaterialImporter();

    private:

        static const SMaterialDescriptor s_DefaultDescriptor;
    };
} // namespace 

namespace 
{
    const SMaterialDescriptor CCoreMaterialImporter::s_DefaultDescriptor =
    {
        "", "", "", "", "", "", "",
        1.0f, 0.0f, 0.0f, 0.0f,
        glm::vec3(1.0f),
        glm::vec4(1.0f, 1.0f, 0.0f, 0.0f)
    };
} // namespace 

namespace 
{
    CCoreMaterialImporter::CCoreMaterialImporter()
    {
    }

    // -----------------------------------------------------------------------------

    CCoreMaterialImporter::~CCoreMaterialImporter()
    {

    }

    // -----------------------------------------------------------------------------

    SMaterialDescriptor CCoreMaterialImporter::CreateDescriptionFromXML(const std::string& _rPathToFile) const
    {
        // -----------------------------------------------------------------------------
        // Descriptor
        // -----------------------------------------------------------------------------
        SMaterialDescriptor MaterialDescriptor = s_DefaultDescriptor;

        // -----------------------------------------------------------------------------
        // Load data
        // -----------------------------------------------------------------------------
        auto Importer = Core::AssetImporter::AllocateTinyXMLImporter(_rPathToFile);

        if (Importer == nullptr) return MaterialDescriptor;

        tinyxml2::XMLDocument* pMaterialFile = static_cast<tinyxml2::XMLDocument*>(Core::AssetImporter::GetNativeAccessFromImporter(Importer));

        if (pMaterialFile == nullptr) return MaterialDescriptor;

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
        MaterialDescriptor.m_MaterialName = pElementDefinition->Attribute("Name");

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

        return MaterialDescriptor;
    }

    // -----------------------------------------------------------------------------

    SMaterialDescriptor CCoreMaterialImporter::CreateDescriptionFromAssimpFile(const std::string& _rPathToFile, int _MaterialIndex) const
    {
        // -----------------------------------------------------------------------------
        // Descriptor
        // -----------------------------------------------------------------------------
        SMaterialDescriptor MaterialDescriptor = s_DefaultDescriptor;

        // -----------------------------------------------------------------------------
        // Importer
        // -----------------------------------------------------------------------------
        auto Importer = Core::AssetImporter::AllocateAssimpImporter(_rPathToFile, Core::AssetImporter::SGeneratorFlag::Nothing);

        if (Importer == nullptr) return MaterialDescriptor;

        const Assimp::Importer* pImporter = static_cast<const Assimp::Importer*>(Core::AssetImporter::GetNativeAccessFromImporter(Importer));

        if (pImporter == nullptr) return MaterialDescriptor;

        const aiScene* pScene = pImporter->GetScene();

        if (pScene == nullptr) return MaterialDescriptor;

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
            // -----------------------------------------------------------------------------
            // Values and textures
            // -----------------------------------------------------------------------------
            aiString  NativeString;
            aiColor4D DiffuseColor;

            if (pMaterial->Get(AI_MATKEY_NAME, NativeString) == AI_SUCCESS)
            {
                MaterialDescriptor.m_MaterialName = NativeString.data;
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

                    return CreateDescriptionFromXML(PathToMat);
                }
            }
        }

        return MaterialDescriptor;
    }
} // namespace 

namespace Core
{
namespace MaterialImporter
{
    SMaterialDescriptor CreateDescriptionFromXML(const std::string& _rPathToFile)
    {
        return CCoreMaterialImporter::GetInstance().CreateDescriptionFromXML(_rPathToFile);
    }

    // -----------------------------------------------------------------------------

    SMaterialDescriptor CreateDescriptionFromAssimpFile(const std::string& _rPathToFile, int _MaterialIndex)
    {
        return CCoreMaterialImporter::GetInstance().CreateDescriptionFromAssimpFile(_rPathToFile, _MaterialIndex);
    }
} // namespace MaterialImporter
} // namespace Core