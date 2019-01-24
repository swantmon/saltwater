
#include "editor_imgui/edit_precompiled.h"

#include "editor_imgui/edit_gui_factory.h"
#include "editor_imgui/edit_scene_graph_panel.h"

#include "engine/data/data_map.h"
#include "engine/data/data_entity.h"
#include "engine/data/data_entity_manager.h"
#include "engine/data/data_hierarchy_facet.h"

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

                if (pHierarchyFacet->GetFirstChild() == 0)
                {
                    ImGui::Indent();

                    ImGui::Selectable(pSibling->GetName().c_str());

                    ImGui::Unindent();
                }
                else
                {
                    if (ImGui::TreeNode("Test", "%s", pSibling->GetName().c_str()))
                    {
                        if (pHierarchyFacet->GetFirstChild() != 0)
                        {
                            RecursiveTree(pHierarchyFacet->GetFirstChild());
                        }

                        ImGui::TreePop();
                    }
                }

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