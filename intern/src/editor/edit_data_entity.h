
#pragma once

#include "engine/data/data_entity.h"

#include "editor/imgui/imgui.h"

#include <vector>

namespace Dt
{
    class CEntityGUI : public CEntity
    {
    public:
        
        bool OnGUI()
        {
            bool HasChanged = false;

            ImGui::Columns(2, nullptr, false);

            ImGui::SetColumnWidth(0, 80);

            // -----------------------------------------------------------------------------
            // ID
            // -----------------------------------------------------------------------------
            ImGui::Text("ID: %i", m_ID);

            // -----------------------------------------------------------------------------
            // Active
            // -----------------------------------------------------------------------------
            bool IsActive = m_Flags.m_IsActive;

            HasChanged |= ImGui::Checkbox("Active", &IsActive);

            SetActive(IsActive);

            ImGui::NextColumn();

            // -----------------------------------------------------------------------------
            // Name
            // -----------------------------------------------------------------------------
            static char EntityName[255];

            strcpy_s(EntityName, m_Name.c_str());

            if (ImGui::InputText("Name", EntityName, 255))
            {
                m_Name = EntityName;

                HasChanged = true;
            }

            // -----------------------------------------------------------------------------
            // Category
            // -----------------------------------------------------------------------------
            const char* CategoryText[SEntityCategory::NumberOfCategories] = { "Static", "Dynamic" };

            auto CategoryIndex = static_cast<int>(GetCategory());

            HasChanged |= ImGui::Combo("Category", &CategoryIndex, CategoryText, SEntityCategory::NumberOfCategories);

            SetCategory(static_cast<SEntityCategory::Enum>(CategoryIndex));

            // -----------------------------------------------------------------------------
            // Layer
            // -----------------------------------------------------------------------------
            unsigned int LayerFlags = m_Flags.m_Layer;
            const char* pCurrentText = LayerFlags > 0 ? "<Multiple Values>" : "<Nothing>";

            const char* EntityLayerText[SEntityLayer::NumberOfLayers] = { 
                "Default", 
                "AR", 
                "Shadow Only",
                "IgnoreRaycast", 
                "UI" 
            };

            const SEntityLayer::Enum EntityLayerValue[SEntityLayer::NumberOfLayers] = { 
                SEntityLayer::Default, 
                SEntityLayer::AR, 
                SEntityLayer::ShadowOnly,
                SEntityLayer::IgnoreRaycast,
                SEntityLayer::UI 
            };
            
            for (int i = 0; i < SEntityLayer::NumberOfLayers; ++i)
            {
                if ((1 << i) == static_cast<int>(LayerFlags))
                {
                    pCurrentText = EntityLayerText[i];
                    break;
                }
            }

            if (ImGui::BeginCombo("Layer", pCurrentText, 0))
            {
                for (int n = 0; n < SEntityLayer::NumberOfLayers; n++)
                {
                    HasChanged |= ImGui::CheckboxFlags(EntityLayerText[n], &LayerFlags, EntityLayerValue[n]);
                }
                ImGui::EndCombo();
            }

            m_Flags.m_Layer = LayerFlags;

            ImGui::Columns(1, nullptr, false);

            return HasChanged;
        }

        // -----------------------------------------------------------------------------

        void OnDropAsset(const Edit::CAsset&)
        {
        }
    };
} // namespace Dt
