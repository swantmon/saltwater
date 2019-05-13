
#include "editor/edit_precompiled.h"

#include "engine/core/core_console.h"

#include "editor/edit_assets_panel.h"
#include "editor/edit_gui_factory.h"
#include "editor/edit_scene_graph_panel.h"

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
    CAssetsPanel::CAssetsPanel()
    {

    }

    // -----------------------------------------------------------------------------

    CAssetsPanel::~CAssetsPanel()
    {

    }

    // -----------------------------------------------------------------------------

    void CAssetsPanel::Render()
    {
        ImGui::SetNextWindowPos(ImVec2(30, 100), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiCond_FirstUseEver);

        ImGui::Begin("Assets", &m_IsVisible);

        

        ImGui::End();
    }

    // -----------------------------------------------------------------------------

    const char* CAssetsPanel::GetName()
    {
        return "Assets";
    }
} // namespace GUI
} // namespace Edit