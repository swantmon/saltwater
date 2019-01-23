
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
            int ID = static_cast<int>(m_ID);

            ImGui::InputInt("ID", &ID);

            CEntity::m_ID = ID;
        }
    };
} // namespace Dt
