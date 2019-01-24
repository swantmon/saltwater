
#pragma once

#include "engine/data/data_entity.h"

#include "editor_imgui/imgui/imgui.h"

namespace Dt
{
    class CEntityGUI : public CEntity
    {
    public:
        
        void OnGUI()
        {
            ImGui::Text("Name: %s", m_Name.c_str());

            ImGui::Text("ID: %i", m_ID);
        }

        // -----------------------------------------------------------------------------

        const char* GetHeader()
        {
            return "Entity";
        }
    };
} // namespace Dt
