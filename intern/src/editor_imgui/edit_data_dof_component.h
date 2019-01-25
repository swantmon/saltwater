
#pragma once

#include "engine/data/data_dof_component.h"

#include "editor_imgui/imgui/imgui.h"

namespace Dt
{
    class CDOFComponentGUI : public CDOFComponent
    {
    public:

        void OnGUI()
        {
            ImGui::DragFloat("Near Distance", &m_NearDistance, 0.01f);

            ImGui::DragFloat("Far Distance", &m_FarDistance, 0.01f);

            ImGui::DragFloat("Near to far ratio", &m_NearToFarRatio, 0.01f);

            ImGui::DragFloat("Fade Un- to Small blur", &m_FadeUnToSmallBlur, 0.01f);

            ImGui::DragFloat("Fade Small- to medium blur", &m_FadeSmallToMediumBlur, 0.01f);

            UpdateEffect();
        }

        // -----------------------------------------------------------------------------

        const char* GetHeader()
        {
            return "Depth-of-Field";
        }
    };
} // namespace Dt
