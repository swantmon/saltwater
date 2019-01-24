
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
            ImGui::BeginGroup();

            ImGui::Text("Name:");

            ImGui::Text(m_Name.c_str());

            ImGui::EndGroup();
        }

        // -----------------------------------------------------------------------------

        const char* GetHeader()
        {
            return "Entity";
        }
    };
} // namespace Dt
