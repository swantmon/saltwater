
#include "editor_imgui/edit_precompiled.h"

#include "base/base_include_glm.h"

#include "editor_imgui/edit_infos_panel.h"

#include "editor_imgui/imgui/imgui.h"

#include "engine/core/core_time.h"

#include "engine/graphic/gfx_main.h"

namespace Edit
{
namespace GUI
{
    CInfosPanel::CInfosPanel()
        : m_FrameTimings()
    {

    }

    // -----------------------------------------------------------------------------

    CInfosPanel::~CInfosPanel()
    {
        m_FrameTimings.clear();
    }

    // -----------------------------------------------------------------------------

    void CInfosPanel::Render()
    {
        ImGuiWindowFlags Style =
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoFocusOnAppearing | 
            ImGuiWindowFlags_NoNav;

        float DeltaTimeLastFrame = static_cast<float>(Core::Time::GetDeltaTimeLastFrame());

        m_FrameTimings.push_back(1.0f / DeltaTimeLastFrame);

        if (m_FrameTimings.size() > 120)
        {
            m_FrameTimings.erase(m_FrameTimings.begin());
        }

        ImGui::SetNextWindowBgAlpha(0.4f);

        ImGui::Begin("Infos", &m_IsVisible, Style);

        ImGui::PlotLines("FPS", m_FrameTimings.data(), m_FrameTimings.size(), 0, 0, 1, 300);

        ImGui::Text("Frequency is %.2f ms/frame (%.0f FPS).", DeltaTimeLastFrame * 1000, 1.0f / glm::max(DeltaTimeLastFrame, 0.0001f));

        ImGui::SetWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - ImGui::GetWindowWidth() - 20, 40), true);

        ImGui::End();
    }

    // -----------------------------------------------------------------------------

    const char* CInfosPanel::GetName()
    {
        return "Infos";
    }
} // namespace GUI
} // namespace Edit