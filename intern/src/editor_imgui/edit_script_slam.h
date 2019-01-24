
#pragma once

#include "engine/script/script_slam.h"

#include "imgui.h"

namespace Edit
{
    class CSLAMScriptGUI : public Scpt::CSLAMScript
    {
    public:

        void OnGUI()
        {
            ImGui::CollapsingHeader("SLAM (Script)");

            ImGui::Checkbox("Active", &m_IsSelectionEnabled);

            ImGui::Separator();
        }

        // -----------------------------------------------------------------------------

        const char* GetHeader()
        {
            return "SLAM (Script)";
        }
    };
} // namespace Edit
