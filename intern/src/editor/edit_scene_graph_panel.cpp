
#include "editor/edit_precompiled.h"

#include "editor/edit_asset_helper.h"
#include "editor/edit_gui_factory.h"
#include "editor/edit_load_map_state.h"
#include "editor/edit_scene_graph_panel.h"
#include "editor/edit_inspector_panel.h"

#include "engine/core/core_console.h"

#include "engine/data/data_map.h"
#include "engine/data/data_entity.h"
#include "engine/data/data_entity_manager.h"
#include "engine/data/data_hierarchy_facet.h"

#include "engine/graphic/gfx_highlight_renderer.h"

#include "imgui/imgui.h"

namespace Edit
{
namespace GUI
{
    CSceneGraphPanel::CSceneGraphPanel()
    {

    }

    // -----------------------------------------------------------------------------

    CSceneGraphPanel::~CSceneGraphPanel()
    {

    }

    // -----------------------------------------------------------------------------

    void CSceneGraphPanel::Render()
    {
        // -----------------------------------------------------------------------------
        // Build graph
        // -----------------------------------------------------------------------------
        m_ItemState.clear();

        std::function<void(Dt::CEntity*, int&)> RecursiveTree = [&](Dt::CEntity* _pEntity, int& _rDepth)->void
        {
            Dt::CEntity* pSibling = _pEntity;

            while (pSibling != nullptr)
            {
                SItemState ItemState;

                ItemState.Depth      = _rDepth;
                ItemState.pEntity    = pSibling;

                m_ItemState.push_back(ItemState);

                auto pHierarchyFacet = pSibling->GetHierarchyFacet();

                if (pHierarchyFacet->GetFirstChild() != nullptr)
                {
                    ++ _rDepth;

                    RecursiveTree(pHierarchyFacet->GetFirstChild(), _rDepth);

                    -- _rDepth;
                }

                pSibling = pHierarchyFacet->GetSibling();
            }
        };

        auto CurrentEntity = Dt::Map::EntitiesBegin();
        auto EndOfEntities = Dt::Map::EntitiesEnd();

        int CurrentDepth = 0;

        while (CurrentEntity != EndOfEntities)
        {
            auto pHierarchyFacet = CurrentEntity->GetHierarchyFacet();

            Dt::CEntity* pEntity = &*CurrentEntity;

            if (pHierarchyFacet->GetParent() == nullptr)
            {
                RecursiveTree(pEntity, CurrentDepth);
            }

            CurrentEntity = CurrentEntity.Next();
        }

        // -----------------------------------------------------------------------------
        // GUI
        // -----------------------------------------------------------------------------
        auto Filename = Edit::CLoadMapState::GetInstance().GetFilename();

        auto Scenename = Filename.substr(0, Filename.find_last_of('.'));

        ImGui::SetNextWindowPos(ImVec2(30, 100), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiCond_FirstUseEver);

        ImGui::Begin(Scenename.c_str(), &m_IsVisible);

        ImGui::BeginChild("SCENE_GRAPH_PANEL_CHILD");

        int MaximumDepth = 1000;

        for (auto[Depth, pEntity] : m_ItemState)
        {
            if (Depth > MaximumDepth) continue;

            MaximumDepth = 1000;

            Dt::CEntity::BID CurrentID = pEntity->GetID();

            ImGui::PushID(static_cast<int>(CurrentID));

            for (int i = 0; i < Depth; ++ i)
            {
                ImGui::Indent();
            }

            auto pHierarchyFacet = pEntity->GetHierarchyFacet();

            if (pHierarchyFacet->GetFirstChild() != nullptr)
            {
                char* Identifier = m_SelectionState[CurrentID] ? "+" : "-";

                if (ImGui::Button(Identifier))
                {
                    m_SelectionState[CurrentID] = !m_SelectionState[CurrentID];
                }

                ImGui::SameLine();
            }

            if (pHierarchyFacet->GetFirstChild() == nullptr)
            {
                ImGui::Indent();
            }

            if (m_SelectionState[CurrentID]) MaximumDepth = Depth;

            if (ImGui::Selectable(pEntity->GetName().c_str()))
            {
                CInspectorPanel::GetInstance().InspectEntity(CurrentID);

                Gfx::HighlightRenderer::HighlightEntity(CurrentID);
            }

            if (ImGui::BeginPopupContextItem())
            {
                if (ImGui::Button("Delete"))
                {
                    Dt::CEntityManager::GetInstance().MarkEntityAsDirty(*pEntity, Dt::CEntity::DirtyRemove | Dt::CEntity::DirtyDestroy);
                }

                ImGui::EndPopup();
            }

            if (pHierarchyFacet->GetFirstChild() == nullptr)
            {
                ImGui::Unindent();
            }

            for (int i = 0; i < Depth; ++i)
            {
                ImGui::Unindent();
            }

            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
            {
                ImGui::SetDragDropPayload("SCENE_GRAPH_DRAGDROP", &CurrentID, sizeof(Dt::CEntity::BID));

                ImGui::Text("%s", pEntity->GetName().c_str());

                ImGui::EndDragDropSource();
            }

            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SCENE_GRAPH_DRAGDROP"))
                {
                    assert(payload->DataSize == sizeof(Dt::CEntity::BID*));

                    auto EntityIDDestination = *static_cast<const Dt::CEntity::BID*>(payload->Data);

                    Dt::CEntity* pSourceEntity = Dt::CEntityManager::GetInstance().GetEntityByID(EntityIDDestination);

                    if (pSourceEntity == nullptr) return;

                    pSourceEntity->Detach();

                    Dt::CEntity* pDestinationEntity = Dt::CEntityManager::GetInstance().GetEntityByID(CurrentID);

                    pDestinationEntity->Attach(*pSourceEntity);

                    Dt::CEntityManager::GetInstance().MarkEntityAsDirty(*pSourceEntity, Dt::CEntity::DirtyMove);
                }
                ImGui::EndDragDropTarget();
            }

            ImGui::PopID();
        }

        ImGui::EndChild();

        if (ImGui::BeginDragDropTarget())
        {
            ImGuiDragDropFlags ImGuiTargetFlags = 0;

            // ImGuiTargetFlags |= ImGuiDragDropFlags_AcceptBeforeDelivery;    // Don't wait until the delivery (release mouse button on a target) to do something
            // ImGuiTargetFlags |= ImGuiDragDropFlags_AcceptNoDrawDefaultRect; // Don't display the yellow rectangle

            if (const ImGuiPayload* _pPayload = ImGui::AcceptDragDropPayload("ASSETS_DRAGDROP", ImGuiTargetFlags))
            {
                auto& DraggedAsset = *static_cast<CAsset*>(_pPayload->Data);

                auto pEntity = Edit::AssetHelper::LoadPrefabFromModel(DraggedAsset);

                if (pEntity != nullptr)
                {
                    Dt::CEntityManager::GetInstance().MarkEntityAsDirty(*pEntity, Dt::CEntity::DirtyCreate | Dt::CEntity::DirtyAdd);
                }
            }

            ImGui::EndDragDropTarget();
        }

        ImGui::End();
    }

    // -----------------------------------------------------------------------------

    const char* CSceneGraphPanel::GetName()
    {
        return "Scene Graph";
    }
} // namespace GUI
} // namespace Edit