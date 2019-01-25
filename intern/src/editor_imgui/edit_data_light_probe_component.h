
#pragma once

#include "engine/data/data_light_probe_component.h"

#include "editor_imgui/imgui/imgui.h"

namespace Dt
{
    class CLightProbeComponentGUI : public CLightProbeComponent
    {
    public:

        void OnGUI()
        {
            // -----------------------------------------------------------------------------
            // Refresh mode
            // -----------------------------------------------------------------------------
            {
                const char* Text[] = { "Static", "Dynamic" };

                int Index = static_cast<int>(GetRefreshMode());

                ImGui::Combo("Refresh Mode", &Index, Text, 2);

                SetRefreshMode(static_cast<ERefreshMode>(Index));
            }

            // -----------------------------------------------------------------------------
            // Type
            // -----------------------------------------------------------------------------
            {
                const char* Text[] = { "Sky", "Local", "Custom" };

                int Index = static_cast<int>(GetType());

                ImGui::Combo("Type", &Index, Text, 3);

                SetType(static_cast<EType>(Index));
            }

            // -----------------------------------------------------------------------------
            // Quality
            // -----------------------------------------------------------------------------
            {
                const char* Text[] = { "32", "64", "128", "256", "512", "1024", "2048" };

                int Index = static_cast<int>(GetQuality());

                ImGui::Combo("Quality", &Index, Text, 7);

                SetQuality(static_cast<EQuality>(Index));
            }

            // -----------------------------------------------------------------------------
            // Clear flag
            // -----------------------------------------------------------------------------
            {
                const char* Text[] = { "Skybox", "Black" };

                int Index = static_cast<int>(GetClearFlag());

                ImGui::Combo("Clear Flag", &Index, Text, 2);

                SetClearFlag(static_cast<EClearFlag>(Index));
            }

            // -----------------------------------------------------------------------------
            // Rest
            // -----------------------------------------------------------------------------
            ImGui::DragFloat("Intensity", &m_Intensity);

            ImGui::DragFloat("Near", &m_Near);

            ImGui::DragFloat("Far", &m_Far);

            ImGui::Checkbox("Parallax Correction", &m_ParallaxCorrection);

            ImGui::DragFloat3("Box Size", &m_BoxSize.x);
        }

        // -----------------------------------------------------------------------------

        const char* GetHeader()
        {
            return "Light Probe";
        }
    };
} // namespace Dt
