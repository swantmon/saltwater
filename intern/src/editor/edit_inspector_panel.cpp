
#include "editor/edit_precompiled.h"

#include "base/base_include_glm.h"

#include "editor/edit_asset_helper.h"
#include "editor/edit_component_factory.h"
#include "editor/edit_edit_state.h"
#include "editor/edit_gui_factory.h"
#include "editor/edit_inspector_panel.h"

#include "editor/imgui/imgui.h"

#include "engine/data/data_component_facet.h"
#include "engine/data/data_component_manager.h"
#include "engine/data/data_entity.h"
#include "engine/data/data_entity_manager.h"
#include "engine/data/data_material_component.h"
#include "engine/data/data_script_component.h"
#include "engine/data/data_transformation_facet.h"

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
            m_pEntity = nullptr;
            return;
        }

        m_pEntity = Dt::CEntityManager::GetInstance().GetEntityByID(_ID);
    }

    // -----------------------------------------------------------------------------

    void CInspectorPanel::Render()
    {
        ImGui::SetNextWindowPos(ImVec2(950, 100), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiCond_FirstUseEver);

        int IndexID = 0;
        bool HasChanged = false;

        auto& rFactory = Edit::CGUIFactory::GetInstance();
        auto& rComponentFactory = Edit::CComponentFactory::GetInstance();

        ImGui::Begin("Inspector", &m_IsVisible);

        if (m_pEntity)
        {
            auto TypeInfo = Base::CTypeInfo::Get(m_pEntity);

            if (rFactory.Has<Dt::CEntity>())
            {
                ImGui::PushID(IndexID);

                auto Panel = rFactory.Get<Dt::CEntity>(m_pEntity);

                if (Panel->OnGUI())
                {
                    CEditState::GetInstance().SetDirty();
                }

                ImGui::PopID();

                ++IndexID;
            }

            ImGui::Separator();

            auto pTransformationFacet = m_pEntity->GetTransformationFacet();

            TypeInfo = Base::CTypeInfo::Get(pTransformationFacet);

            if (pTransformationFacet && rFactory.Has<Dt::CTransformationFacet>())
            {
                auto Panel = rFactory.Get<Dt::CTransformationFacet>(pTransformationFacet);

                ImGui::PushID(IndexID);

                if (ImGui::CollapsingHeader(Panel->GetHeader(), ImGuiTreeNodeFlags_DefaultOpen))
                {
                    if (Panel->OnGUI()) CEditState::GetInstance().SetDirty();
                }

                ImGui::PopID();

                ++IndexID;
            }

            auto pComponentFacet = m_pEntity->GetComponentFacet();

            if (pComponentFacet)
            {
                for (auto pComponent : pComponentFacet->GetComponents())
                {
                    TypeInfo = pComponent->GetTypeInfo();

                    if (pComponent->GetTypeInfo() == Base::CTypeInfo::Get<Dt::CScriptComponent>())
                    {
                        auto pScriptComponent = static_cast<Dt::CScriptComponent*>(pComponent);

						TypeInfo = pScriptComponent->GetScriptTypeInfo();
                    }

                    if (rFactory.Has(TypeInfo))
                    {
                        auto Panel = rFactory.Get(TypeInfo, pComponent);

                        ImGui::PushID(IndexID);
                        
                        if (ImGui::CollapsingHeader(Panel->GetHeader()))
                        {
                            bool IsActive = pComponent->IsActive();

                            if (ImGui::Checkbox("Active##ComponentActive", &IsActive))
                            {
                                pComponent->SetActive(IsActive);

                                HasChanged = true;
                            }                            

                            if (Panel->OnGUI())
                            {
                                Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pComponent, Dt::IComponent::DirtyInfo);

								HasChanged = true;
							}
                        }

                        ImGui::PopID();

                        if (ImGui::BeginDragDropTarget())
                        {
                            if (const ImGuiPayload * _pPayload = ImGui::AcceptDragDropPayload("ASSETS_DRAGDROP", 0))
                            {
                                auto& DraggedAsset = *static_cast<Edit::CAsset*>(_pPayload->Data);

                                Panel->OnDropAsset(DraggedAsset);

								Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pComponent, Dt::IComponent::DirtyInfo);
                            }

                            ImGui::EndDragDropTarget();
                        }

                        ++IndexID;
                    }
                }
            }

            Dt::CEntityManager::GetInstance().MarkEntityAsDirty(*m_pEntity, Dt::CEntity::DirtyMove);

            ImGui::Separator();

            ImGui::PushItemWidth(-1);

            if (ImGui::BeginCombo("##ADD_COMPONENT", "Add Component"))
            {
                ImGui::PushItemWidth(-1);

                static ImGuiTextFilter TextFilter;

                TextFilter.Draw("##SEARCH_REGEX");

                ImGui::PopItemWidth();

                for (auto pComponent : rComponentFactory.GetComponents())
                {
                    if (!TextFilter.PassFilter(pComponent->GetHeader())) continue;

                    if (ImGui::Selectable(pComponent->GetHeader()))
                    {
                        pComponent->OnNewComponent(m_pEntity->GetID());
                    }
                }

                ImGui::EndCombo();
            }

            ImGui::PopItemWidth();

            // -----------------------------------------------------------------------------
            // Special issues
            // -----------------------------------------------------------------------------
            if (pComponentFacet && pComponentFacet->HasComponent<Dt::CMaterialComponent>() == false)
            {
                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload * _pPayload = ImGui::AcceptDragDropPayload("ASSETS_DRAGDROP", 0))
                    {
                        auto& DraggedAsset = *static_cast<Edit::CAsset*>(_pPayload->Data);

                        if (DraggedAsset.GetType() == Edit::CAsset::Material)
                        {
                            auto pComponent = Dt::CComponentManager::GetInstance().Allocate<Dt::CMaterialComponent>();

                            pComponent->SetFileName(DraggedAsset.GetPathToFile());

                            m_pEntity->AttachComponent(pComponent);

                            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pComponent, Dt::CMaterialComponent::DirtyCreate);

                            HasChanged = true;
                        }
                    }

                    ImGui::EndDragDropTarget();
                }
            }

            // -----------------------------------------------------------------------------
            // Dirty
            // -----------------------------------------------------------------------------
            if (HasChanged)
            {
                CEditState::GetInstance().SetDirty();
            }
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