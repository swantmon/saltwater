
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
            ImGui::Text("SLAM script Component");
        }
    };
} // namespace Edit
