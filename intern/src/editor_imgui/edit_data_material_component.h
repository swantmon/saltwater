
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
            ImGui::Text("Material Component");

            ImGui::ColorEdit4("Albedo", &m_Color.x);
            ImGui::DragFloat("Roughness", &m_Roughness);
        }
    };
} // namespace Dt
