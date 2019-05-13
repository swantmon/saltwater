
#include "editor/edit_precompiled.h"

#include "base/base_include_glm.h"

#include "editor/edit_toolbar_panel.h"

#include "editor/edit_edit_state.h"
#include "editor/imgui/imgui.h"
#include "editor/imgui/extensions/ImGuizmo.h"

#include "engine/core/core_time.h"

#include "engine/graphic/gfx_main.h"

namespace Edit
{
namespace GUI
{
    CToolbarPanel::CToolbarPanel()
        : m_FrameTimings()
    {

    }

    // -----------------------------------------------------------------------------

    CToolbarPanel::~CToolbarPanel()
    {
        m_FrameTimings.clear();
    }

    // -----------------------------------------------------------------------------

    void CToolbarPanel::Render()
    {
        ImGui::Begin("Toolbar", &m_IsVisible);

        auto Operation = Edit::CEditState::GetInstance().GetOperation();
        auto Mode      = Edit::CEditState::GetInstance().GetMode();

        if (ImGui::RadioButton("Hand", Operation == Edit::CEditState::Hand))
        {
            Operation = Edit::CEditState::Hand;
        }

        ImGui::SameLine();

        if (ImGui::RadioButton("Translate", Operation == Edit::CEditState::Translate))
        {
            Operation = Edit::CEditState::Translate;
        }

        ImGui::SameLine();

        if (ImGui::RadioButton("Rotate", Operation == Edit::CEditState::Rotate))
        {
            Operation = Edit::CEditState::Rotate;
        }

        ImGui::SameLine();

        if (ImGui::RadioButton("Scale", Operation == Edit::CEditState::Scale))
        {
            Operation = Edit::CEditState::Scale;
        }

        ImGui::SameLine();

        if (ImGui::RadioButton("Local", Mode == Edit::CEditState::Local))
        {
            Mode = Edit::CEditState::Local;
        }

        ImGui::SameLine();

        if (ImGui::RadioButton("World", Mode == Edit::CEditState::World))
        {
            Mode = Edit::CEditState::World;
        }

        Edit::CEditState::GetInstance().SetOperation(Operation);

        Edit::CEditState::GetInstance().SetMode(Mode);

        ImGui::SetWindowPos(ImVec2(20, 40), true);

        ImGui::End();
    }

    // -----------------------------------------------------------------------------

    const char* CToolbarPanel::GetName()
    {
        return "Toolbar";
    }
} // namespace GUI
} // namespace Edit