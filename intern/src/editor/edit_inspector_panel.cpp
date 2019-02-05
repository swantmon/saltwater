
#include "editor/edit_precompiled.h"

#include "base/base_include_glm.h"

#include "editor/edit_component_factory.h"
#include "editor/edit_gui_factory.h"
#include "editor/edit_inspector_panel.h"

#include "editor/imgui/imgui.h"

#include "engine/data/data_entity.h"
#include "engine/data/data_entity_manager.h"
#include "engine/data/data_transformation_facet.h"
#include "engine/data/data_component_facet.h"
#include "engine/data/data_component_manager.h"
#include "engine/data/data_script_component.h"

#include <regex>

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
        ImGui::SetNextWindowPos(ImVec2(950, 100), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiCond_FirstUseEver);

        int IndexID = 0;
        Edit::CGUIFactory& rFactory = Edit::CGUIFactory::GetInstance();
        Edit::CComponentFactory& rComponentFactory = Edit::CComponentFactory::GetInstance();

        ImGui::Begin("Inspector", &m_IsVisible);

        if (m_pEntity)
        {
            auto Hash = rFactory.CalculateHash(m_pEntity);

            if (rFactory.Has<Dt::CEntity>())
            {
                ImGui::PushID(IndexID);

                auto Panel = rFactory.Get<Dt::CEntity>(m_pEntity);

                Panel->OnGUI();

                ImGui::PopID();

                ++IndexID;
            }

            ImGui::Separator();

            auto pTransformationFacet = m_pEntity->GetTransformationFacet();

            Hash = rFactory.CalculateHash(pTransformationFacet);

            if (pTransformationFacet && rFactory.Has<Dt::CTransformationFacet>())
            {
                auto Panel = rFactory.Get<Dt::CTransformationFacet>(pTransformationFacet);

                ImGui::PushID(IndexID);

                if (ImGui::CollapsingHeader(Panel->GetHeader(), ImGuiTreeNodeFlags_DefaultOpen))
                {
                    Panel->OnGUI();
                }

                ImGui::PopID();

                ++IndexID;
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

                        ImGui::PushID(IndexID);

                        if (ImGui::CollapsingHeader(Panel->GetHeader()))
                        {
                            Panel->OnGUI();
                        }

                        ImGui::PopID();

                        ++IndexID;
                    }

                    Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pComponent, Dt::IComponent::DirtyInfo);
                }
            }

            Dt::EntityManager::MarkEntityAsDirty(*m_pEntity, Dt::CEntity::DirtyMove);

            ImGui::Separator();

            ImGui::PushItemWidth(-1);

            char SearchCharBuffer[64];

            strcpy_s<64>(SearchCharBuffer, m_SearchString.c_str());

            if (ImGui::BeginCombo("##ADD_COMPONENT", "Add Component"))
            {
                ImGui::PushItemWidth(-1);

                if (ImGui::InputText("##SEARCH_REGEX", SearchCharBuffer, 64))
                {
                    m_SearchString = SearchCharBuffer;
                }

                ImGui::PopItemWidth();

                for (auto pComponent : rComponentFactory.GetComponents())
                {
                    if (m_SearchString.length() == 0 || std::regex_match(pComponent->GetHeader(), std::regex("(.*)(" + m_SearchString + ")(.*)")))
                    {
                        if (ImGui::Selectable(pComponent->GetHeader()))
                        {
                            pComponent->OnNewComponent(m_pEntity->GetID());
                        }
                    }
                }

                ImGui::EndCombo();
            }
            else
            {
                m_SearchString = "";
            }

            ImGui::PopItemWidth();
        }

        ImGui::End();
    }

    // -----------------------------------------------------------------------------

    const char* CInspectorPanel::GetName()
    {
        return "Inspector";
    }
} // namespace GUI
} // namespace Edit