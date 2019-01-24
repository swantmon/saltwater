
#pragma once

#include "engine/data/data_material_component.h"

#include "imgui.h"

namespace Dt
{
    class CMaterialComponentGUI : public CMaterialComponent
    {
    public:

        void OnGUI()
        {
            ImGui::ColorEdit4("Albedo", &m_Color.x);
            ImGui::SliderFloat("Roughness", &m_Roughness, 0.0f, 1.0f);
        }

        // -----------------------------------------------------------------------------

        const char* GetHeader()
        {
            return "Material";
        }
    };
} // namespace Dt
