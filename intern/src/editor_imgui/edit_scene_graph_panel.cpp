
#include "editor_imgui/edit_precompiled.h"

#include "editor_imgui/edit_gui_factory.h"
#include "editor_imgui/edit_scene_graph_panel.h"

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
        Edit::CGUIFactory& rFactory = Edit::CGUIFactory::GetInstance();

        ImGui::Begin("Scene Graph");

        

        ImGui::End();
    }
} // namespace GUI
} // namespace Edit