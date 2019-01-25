
#pragma once

#include "engine/data/data_ssr_component.h"

#include "editor_imgui/imgui/imgui.h"

namespace Dt
{
    class CSSRComponentGUI : public CSSRComponent
    {
    public:

        void OnGUI()
        {
            ImGui::DragFloat("Intensity", &m_Intensity);

            ImGui::DragFloat("Roughness", &m_RoughnessMask);

            ImGui::DragFloat("Distance", &m_Distance);

            ImGui::Checkbox("Use last frame", &m_UseLastFrame);
        }

        // -----------------------------------------------------------------------------

        const char* GetHeader()
        {
            return "Screen Space Reflections";
        }
    };
} // namespace Dt
