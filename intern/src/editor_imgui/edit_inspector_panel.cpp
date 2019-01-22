
#include "editor_imgui/edit_precompiled.h"

#include "base/base_include_glm.h"

#include "editor_imgui/edit_inspector_panel.h"

#include "imgui.h"

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

    void CInspectorPanel::Render()
    {
        ImGui::Begin("Inspector");



        ImGui::End();
    }
} // namespace GUI
} // namespace Edit