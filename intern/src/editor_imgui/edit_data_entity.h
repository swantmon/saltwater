
#pragma once

#include "engine/data/data_entity.h"

#include "editor_imgui/imgui/imgui.h"

#include <vector>

namespace Dt
{
    class CEntityGUI : public CEntity
    {
    public:
        
        void OnGUI()
        {
            ImGui::Columns(2, nullptr, false);

            ImGui::SetColumnWidth(0, 50);

            // -----------------------------------------------------------------------------
            // ID
            // -----------------------------------------------------------------------------
            ImGui::Text("ID: %i", m_ID);

            ImGui::NextColumn();

            // -----------------------------------------------------------------------------
            // Name
            // -----------------------------------------------------------------------------
            static char EntityName[255];

            strcpy_s(EntityName, m_Name.c_str());

            ImGui::InputText("Name", EntityName, 255);

            m_Name = EntityName;

            // -----------------------------------------------------------------------------
            // Category
            // -----------------------------------------------------------------------------
            const char* CategoryText[SEntityCategory::NumberOfCategories] = { "Static", "Dynamic" };

            int CategoryIndex = static_cast<int>(GetCategory());

            ImGui::Combo("Category", &CategoryIndex, CategoryText, SEntityCategory::NumberOfCategories);

            SetCategory(static_cast<SEntityCategory::Enum>(CategoryIndex));

            // -----------------------------------------------------------------------------
            // Layer
            // -----------------------------------------------------------------------------
            unsigned int LayerFlags = m_Flags.m_Layer;
            const char* item_current = "<Multiple Values>";

            const char* EntityLayerText[SEntityLayer::NumberOfLayers] = { 
                "Default", 
                "AR", 
                "IgnoreRaycast", 
                "UI" 
            };

            const SEntityLayer::Enum EntityLayerValue[SEntityLayer::NumberOfLayers] = { 
                SEntityLayer::Default, 
                SEntityLayer::AR, 
                SEntityLayer::IgnoreRaycast,
                SEntityLayer::UI 
            };
            
            for (int i = 0; i < SEntityLayer::NumberOfLayers; ++i)
            {
                if ((1 << i) == LayerFlags)
                {
                    item_current = EntityLayerText[i];
                    break;
                }
            }

            if (ImGui::BeginCombo("Layer", item_current, 0))
            {
                for (int n = 0; n < SEntityLayer::NumberOfLayers; n++)
                {
                    ImGui::CheckboxFlags(EntityLayerText[n], &LayerFlags, EntityLayerValue[n]);
                }
                ImGui::EndCombo();
            }

            m_Flags.m_Layer = LayerFlags;

            ImGui::Columns(1, nullptr, false);
        }

        // -----------------------------------------------------------------------------

        const char* GetHeader()
        {
            return "Entity";
        }
    };
} // namespace Dt
