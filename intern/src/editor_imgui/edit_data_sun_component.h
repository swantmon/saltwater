
#pragma once

#include "engine/data/data_sun_component.h"

#include "editor_imgui/imgui/imgui.h"

namespace Dt
{
    class CSunComponentGUI : public CSunComponent
    {
    public:

        void OnGUI()
        {
            const char* RefreshModeText[] = { "Static", "Dynamic" };

            int RefreshModeIndex = static_cast<int>(GetRefreshMode());

            ImGui::Combo("Refresh Mode", &RefreshModeIndex, RefreshModeText, 2);

            SetRefreshMode(static_cast<ERefreshMode>(RefreshModeIndex));

            ImGui::ColorEdit3("Color", &m_Color.r);

            ImGui::DragFloat("Intensity", &m_Intensity);

            ImGui::DragFloat3("Direction", &m_Direction.x, 0.001f);

            UpdateLightness();
        }

        // -----------------------------------------------------------------------------

        const char* GetHeader()
        {
            return "Sun";
        }
    };
} // namespace Dt
