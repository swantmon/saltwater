
#include "editor/edit_precompiled.h"

#include "editor/edit_asset_helper.h"

#include "engine/data/data_entity_manager.h"
#include "engine/data/data_map.h"
#include "engine/data/data_mesh_component.h"
#include "engine/data/data_camera_component.h"
#include "engine/data/data_component_manager.h"

#include "engine/graphic/gfx_material_manager.h"

namespace Edit
{
    std::array<std::regex, CAsset::NumberOfTypes> CAsset::s_Filter = { std::regex(".*.(obj|dae|fbx)", std::regex_constants::icase), std::regex(".*.(mat)"), std::regex(".*.(jpg|tga|dds|png|hdr)", std::regex_constants::icase), std::regex(".*.(sws)"), std::regex(".*.(swr)"), std::regex(".*.[]?") };
} // namespace Edit

namespace Edit
{
    CAsset::CAsset(const std::string& _rFilename)
        : m_PathToFile(_rFilename)
        , m_Type(Undefined)
    {
        Identify();
    }

    // -----------------------------------------------------------------------------

    const std::string& CAsset::GetPathToFile() const
    {
        return m_PathToFile;
    }

    // -----------------------------------------------------------------------------

    CAsset::EType CAsset::GetType() const
    {
        return m_Type;
    }

    // -----------------------------------------------------------------------------

    void CAsset::Identify()
    {
        int Index = 0;

        for (auto& rRegex : s_Filter)
        {
            if (std::regex_match(m_PathToFile, rRegex))
            {
                m_Type = static_cast<EType>(Index);

                break;
            }

            ++Index;
        }
    }
} // namespace Edit

namespace Edit
{
namespace AssetHelper
{
    Dt::CEntity* LoadPrefabFromModel(const CAsset& _rAsset)
    {
        if (_rAsset.GetType() != CAsset::Model) return nullptr;

        auto Entities = Dt::CEntityManager::GetInstance().CreateEntitiesFromScene(_rAsset.GetPathToFile());

        Dt::SEntityDescriptor EntityDesc;

        EntityDesc.m_EntityCategory = Dt::SEntityCategory::Dynamic;
        EntityDesc.m_FacetFlags = Dt::CEntity::FacetHierarchy | Dt::CEntity::FacetTransformation | Dt::CEntity::FacetComponents;

        Dt::CEntity& rNewModel = Dt::CEntityManager::GetInstance().CreateEntity(EntityDesc);

        for (auto& rEntity : Entities) rNewModel.Attach(*rEntity);

        rNewModel.SetName("New model");

        return &rNewModel;
    }
} // namespace AssetHelper
} // namespace Edit