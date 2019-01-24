
#include "editor_imgui/edit_precompiled.h"

#include "base/base_include_glm.h"

#include "editor_imgui/edit_inspector_panel.h"
#include "editor_imgui/edit_gui_factory.h"

#include "editor_imgui/imgui/imgui.h"

#include "engine/data/data_entity.h"
#include "engine/data/data_entity_manager.h"
#include "engine/data/data_transformation_facet.h"
#include "engine/data/data_component_facet.h"
#include "engine/data/data_component_manager.h"
#include "engine/data/data_script_component.h"

namespace Edit
{
namespace GUI
{
    CInspectorPanel::CInspectorPanel()
    {

    }

    // -----------------------------------------------------------------------------

    CInspectorPanel::~CInspectorPanel()
    {

    }

    // -----------------------------------------------------------------------------

    void CInspectorPanel::InspectEntity(Dt::CEntity::BID _ID)
    {
        if (_ID == Dt::CEntity::s_InvalidID)
        {
            m_pEntity = 0;
            return;
        }

        m_pEntity = Dt::EntityManager::GetEntityByID(_ID);
    }

    // -----------------------------------------------------------------------------

    void CInspectorPanel::Render()
    {
        Edit::CGUIFactory& rFactory = Edit::CGUIFactory::GetInstance();

        ImGui::Begin("Inspector");

        if (m_pEntity)
        {
            auto Hash = rFactory.CalculateHash(m_pEntity);

            if (rFactory.Has<Dt::CEntity>())
            {
                auto Panel = rFactory.Get<Dt::CEntity>(m_pEntity);

                Panel->OnGUI();
            }

            auto pTransformationFacet = m_pEntity->GetTransformationFacet();

            Hash = rFactory.CalculateHash(pTransformationFacet);

            if (pTransformationFacet && rFactory.Has<Dt::CTransformationFacet>())
            {
                auto Panel = rFactory.Get<Dt::CTransformationFacet>(pTransformationFacet);

                if (ImGui::CollapsingHeader(Panel->GetHeader(), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed))
                {
                    Panel->OnGUI();
                }
            }

            auto pComponentFacet = m_pEntity->GetComponentFacet();

            if (pComponentFacet)
            {
                for (auto pComponent : pComponentFacet->GetComponents())
                {
                    Hash = pComponent->GetTypeID();

                    if (pComponent->GetTypeID() == Base::CTypeInfo::GetTypeID<Dt::CScriptComponent>())
                    {
                        auto pScriptComponent = static_cast<Dt::CScriptComponent*>(pComponent);

                        Hash = pScriptComponent->GetScriptTypeID();
                    }

                    if (rFactory.Has(Hash))
                    {
                        auto Panel = rFactory.Get(Hash, pComponent);

                        if (ImGui::CollapsingHeader(Panel->GetHeader()))
                        {
                            Panel->OnGUI();
                        }
                    }

                    Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pComponent, Dt::IComponent::DirtyInfo);
                }
            }

            Dt::EntityManager::MarkEntityAsDirty(*m_pEntity, Dt::CEntity::DirtyMove);
        }

        ImGui::End();
    }
} // namespace GUI
} // namespace Edit