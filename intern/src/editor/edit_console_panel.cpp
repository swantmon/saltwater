
#include "editor/edit_precompiled.h"

#include "base/base_include_glm.h"

#include "editor/edit_console_panel.h"

#include "editor/imgui/imgui.h"

#include "engine/core/core_time.h"

#include "engine/graphic/gfx_main.h"

namespace Edit
{
namespace GUI
{
    CConsolePanel::CConsolePanel()
    {

    }

    // -----------------------------------------------------------------------------

    CConsolePanel::~CConsolePanel()
    {
    }

    // -----------------------------------------------------------------------------

    void CConsolePanel::Render()
    {
        ImGui::Begin("Console", &m_IsVisible);

        ImGui::End();
    }

    // -----------------------------------------------------------------------------

    const char* CConsolePanel::GetName()
    {
        return "Console";
    }
} // namespace GUI
} // namespace Edit