
#include "editor_imgui/edit_precompiled.h"

#include "editor_imgui/edit_gui_factory.h"
#include "editor_imgui/edit_scene_graph_panel.h"
#include "editor_imgui/edit_inspector_panel.h"

#include "engine/data/data_map.h"
#include "engine/data/data_entity.h"
#include "engine/data/data_entity_manager.h"
#include "engine/data/data_hierarchy_facet.h"

#include "engine/graphic/gfx_selection_renderer.h"

#include "imgui.h"

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
        std::function<void(Dt::CEntity*)> RecursiveTree = [&](Dt::CEntity* _pEntity)->void
        {
            Dt::CEntity* pSibling = _pEntity;

            while (pSibling != nullptr)
            {
                auto pHierarchyFacet = pSibling->GetHierarchyFacet();

                Dt::CEntity::BID CurrentEntityID = pSibling->GetID();

                ImGui::PushID(CurrentEntityID);

                ImGui::Indent();

                // -----------------------------------------------------------------------------
                // Identfier
                // -----------------------------------------------------------------------------
                if (pHierarchyFacet->GetFirstChild() != 0)
                {
                    char* Identifier = m_SelectionState[CurrentEntityID] ? "-" : "+";

                    if (ImGui::Button(Identifier))
                    {
                        m_SelectionState[CurrentEntityID] = !m_SelectionState[CurrentEntityID];
                    }

                    ImGui::SameLine();
                }

                // -----------------------------------------------------------------------------
                // Entity
                // -----------------------------------------------------------------------------
                if (ImGui::Selectable(pSibling->GetName().c_str(), m_SelectionState[CurrentEntityID]))
                {
                    CInspectorPanel::GetInstance().InspectEntity(CurrentEntityID);

                    Gfx::SelectionRenderer::SelectEntity(CurrentEntityID);
                }

                // -----------------------------------------------------------------------------
                // Children
                // -----------------------------------------------------------------------------
                if (m_SelectionState[CurrentEntityID])
                {
                    if (pHierarchyFacet->GetFirstChild() != 0)
                    {
                        RecursiveTree(pHierarchyFacet->GetFirstChild());
                    }
                }

                ImGui::Unindent();

                // -----------------------------------------------------------------------------
                // Drag & Drop
                // -----------------------------------------------------------------------------
                if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
                {
                    ImGui::SetDragDropPayload("DND_DEMO_CELL", &CurrentEntityID, sizeof(Dt::CEntity::BID));
                    ImGui::EndDragDropSource();
                }
                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_DEMO_CELL"))
                    {
                        assert(payload->DataSize == sizeof(Dt::CEntity::BID*));

                        const Dt::CEntity::BID EntityIDDestination = *(const Dt::CEntity::BID*)payload->Data;

                        Dt::CEntity* pSourceEntity = Dt::EntityManager::GetEntityByID(EntityIDDestination);

                        if (pSourceEntity == nullptr) return;

                        pSourceEntity->Detach();

                        Dt::CEntity* pDestinationEntity = Dt::EntityManager::GetEntityByID(CurrentEntityID);

                        pDestinationEntity->Attach(*pSourceEntity);

                        Dt::EntityManager::MarkEntityAsDirty(*pSourceEntity, Dt::CEntity::DirtyMove);
                    }
                    ImGui::EndDragDropTarget();
                }

                ImGui::PopID();

                pSibling = pHierarchyFacet->GetSibling();
            }
        };


        Edit::CGUIFactory& rFactory = Edit::CGUIFactory::GetInstance();

        ImGui::Begin("Scene Graph");

        auto CurrentEntity = Dt::Map::EntitiesBegin();
        auto EndOfEntities = Dt::Map::EntitiesEnd();

        while (CurrentEntity != EndOfEntities)
        {
            auto pHierarchyFacet = CurrentEntity->GetHierarchyFacet();

            Dt::CEntity* pEntity = &*CurrentEntity;

            if (pHierarchyFacet->GetParent() == 0)
            {
                RecursiveTree(pEntity);
            }

            CurrentEntity = CurrentEntity.Next();
        }

        ImGui::End();
    }
} // namespace GUI
} // namespace Edit