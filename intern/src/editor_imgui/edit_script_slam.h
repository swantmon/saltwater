
#pragma once

#include "engine/script/script_slam.h"

#include "editor_imgui/imgui/imgui.h"

namespace Scpt
{
    class CSLAMScriptGUI : public Scpt::CSLAMScript
    {
    public:

        void OnGUI()
        {
             ImGui::Checkbox("Active", &m_IsSelectionEnabled);
        }

        // -----------------------------------------------------------------------------

        const char* GetHeader()
        {
            return "SLAM (Script)";
        }
    };
} // namespace Scpt
