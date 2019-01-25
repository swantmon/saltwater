
#include "editor_imgui/edit_precompiled.h"

#include "engine/core/core_console.h"

#include "editor_imgui/edit_gui_factory.h"
#include "editor_imgui/edit_scene_graph_panel.h"
#include "editor_imgui/edit_inspector_panel.h"

#include "engine/data/data_map.h"
#include "engine/data/data_entity.h"
#include "engine/data/data_entity_manager.h"
#include "engine/data/data_hierarchy_facet.h"

#include "engine/graphic/gfx_selection_renderer.h"

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

                if (pHierarchyFacet->GetFirstChild() != 0)
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

            if (pHierarchyFacet->GetParent() == 0)
            {
                RecursiveTree(pEntity, CurrentDepth);
            }

            CurrentEntity = CurrentEntity.Next();
        }

        // -----------------------------------------------------------------------------
        // GUI
        // -----------------------------------------------------------------------------
        Edit::CGUIFactory& rFactory = Edit::CGUIFactory::GetInstance();

        ImGui::Begin("Scene Graph", &m_IsVisible);

        int MaximumDepth = 1000;

        for (auto& rItemState : m_ItemState)
        {
            if (rItemState.Depth > MaximumDepth) continue;

            MaximumDepth = 1000;

            Dt::CEntity::BID CurrentID = rItemState.pEntity->GetID();

            ImGui::PushID(CurrentID);

            for (int i = 0; i < rItemState.Depth; ++ i)
            {
                ImGui::Indent();
            }

            auto pHierarchyFacet = rItemState.pEntity->GetHierarchyFacet();

            if (pHierarchyFacet->GetFirstChild() != 0)
            {
                char* Identifier = m_SelectionState[CurrentID] ? "+" : "-";

                if (ImGui::Button(Identifier))
                {
                    m_SelectionState[CurrentID] = !m_SelectionState[CurrentID];
                }

                ImGui::SameLine();
            }

            if (pHierarchyFacet->GetFirstChild() == 0)
            {
                ImGui::Indent();
            }

            if (m_SelectionState[CurrentID]) MaximumDepth = rItemState.Depth;

            if (ImGui::Selectable(rItemState.pEntity->GetName().c_str()))
            {
                CInspectorPanel::GetInstance().InspectEntity(CurrentID);

                Gfx::SelectionRenderer::SelectEntity(CurrentID);
            }

            if (ImGui::BeginPopupContextItem())
            {
                if (ImGui::Button("Delete"))
                {
                    Dt::EntityManager::MarkEntityAsDirty(*rItemState.pEntity, Dt::CEntity::DirtyRemove | Dt::CEntity::DirtyDestroy);
                }

                ImGui::EndPopup();
            }

            if (pHierarchyFacet->GetFirstChild() == 0)
            {
                ImGui::Unindent();
            }

            for (int i = 0; i < rItemState.Depth; ++i)
            {
                ImGui::Unindent();
            }

            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
            {
                ImGui::SetDragDropPayload("SCENE_GRAPH_DRAGDROP", &CurrentID, sizeof(Dt::CEntity::BID));

                ImGui::Text("%s", rItemState.pEntity->GetName().c_str());

                ImGui::EndDragDropSource();
            }

            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SCENE_GRAPH_DRAGDROP"))
                {
                    assert(payload->DataSize == sizeof(Dt::CEntity::BID*));

                    const Dt::CEntity::BID EntityIDDestination = *(const Dt::CEntity::BID*)payload->Data;

                    Dt::CEntity* pSourceEntity = Dt::EntityManager::GetEntityByID(EntityIDDestination);

                    if (pSourceEntity == nullptr) return;

                    pSourceEntity->Detach();

                    Dt::CEntity* pDestinationEntity = Dt::EntityManager::GetEntityByID(CurrentID);

                    pDestinationEntity->Attach(*pSourceEntity);

                    Dt::EntityManager::MarkEntityAsDirty(*pSourceEntity, Dt::CEntity::DirtyMove);
                }
                ImGui::EndDragDropTarget();
            }

            ImGui::PopID();
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