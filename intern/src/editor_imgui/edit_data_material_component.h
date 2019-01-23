
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
        }
    };
} // namespace Dt
