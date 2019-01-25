
#pragma once

#include "engine/data/data_bloom_component.h"

#include "editor_imgui/imgui/imgui.h"

namespace Dt
{
    class CBloomComponentGUI : public CBloomComponent
    {
    public:

        void OnGUI()
        {
            ImGui::ColorEdit4("Tint", &m_Tint.x);

            ImGui::DragFloat("Intensity", &m_Intensity, 0.1f, 0.0f, 8.0f);

            ImGui::DragFloat("Threshold", &m_Treshhold, 0.1f, -1.0f, 8.0f);

            ImGui::DragFloat("Exposure Scale", &m_ExposureScale, 0.1f);

            unsigned int Min = 0;
            unsigned int Max = 5;

            ImGui::DragScalar("Scale", ImGuiDataType_U32, &m_Size, 1.0f, &Min, &Max);

            UpdateEffect();
        }

        // -----------------------------------------------------------------------------

        const char* GetHeader()
        {
            return "Bloom";
        }
    };
} // namespace Dt
