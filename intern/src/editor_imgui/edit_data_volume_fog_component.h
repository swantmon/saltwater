
#pragma once

#include "engine/data/data_volume_fog_component.h"

#include "editor_imgui/imgui/imgui.h"

namespace Dt
{
    class CVolumeFogComponentGUI : public CVolumeFogComponent
    {
    public:

        void OnGUI()
        {
            ImGui::DragFloat3("Wind Direction", &m_WindDirection.x);

            ImGui::DragFloat("Wind Speed", &m_WindDirection.w);

            ImGui::DragFloat("Frustum Depth", &m_FrustumDepthInMeter, 1.0f, 0.0f, 0.0f, "%.3f units");

            ImGui::DragFloat("Shadow Intensity", &m_ShadowIntensity);

            ImGui::DragFloat("Scattering Coefficient", &m_ScatteringCoefficient);

            ImGui::DragFloat("Absorption Coefficient", &m_AbsorptionCoefficient);

            ImGui::DragFloat("Density Level", &m_DensityLevel);

            ImGui::DragFloat("Density Attenuation", &m_DensityAttenuation);
        }

        // -----------------------------------------------------------------------------

        const char* GetHeader()
        {
            return "Volumetric Fog";
        }
    };
} // namespace Dt
