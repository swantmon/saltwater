
#pragma once

#include "engine/data/data_entity.h"

#include "imgui.h"

namespace Dt
{
    class CEntityGUI : public CEntity
    {
    public:
        
        void OnGUI()
        {
            ImGui::Text("Name:");

            ImGui::Text(m_Name.c_str());
        }
    };
} // namespace Dt
