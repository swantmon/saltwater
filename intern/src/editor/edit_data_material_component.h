
#pragma once

#include "engine/data/data_material_component.h"

#include "editor/imgui/imgui.h"

namespace Dt
{
    class CMaterialComponentGUI : public CMaterialComponent
    {
    public:

        void OnGUI()
        {
            ImGui::BeginChild("MATERIAL_COMPONENT_GUI_CHILD");

            ImGui::ColorEdit4("Albedo", &m_Color.x);

            ImGui::SliderFloat("Roughness", &m_Roughness, 0.0f, 1.0f);

            ImGui::SliderFloat("Reflectance", &m_Reflectance, 0.0f, 1.0f);

            ImGui::SliderFloat("Metalness", &m_MetalMask, 0.0f, 1.0f);

            ImGui::SliderFloat("Displacement", &m_Displacement, 0.0f, 1.0f);

            ImGui::SliderFloat("Refraction", &m_RefractionIndex, 0.0f, 1.0f);

            ImGui::DragFloat2("Tilling", &m_TilingOffset.x);

            ImGui::DragFloat2("Offset", &m_TilingOffset.z);

            ImGui::EndChild();

            if (ImGui::BeginDragDropTarget())
            {
                ImGuiDragDropFlags ImGuiTargetFlags = 0;

                // ImGuiTargetFlags |= ImGuiDragDropFlags_AcceptBeforeDelivery;    // Don't wait until the delivery (release mouse button on a target) to do something
                // ImGuiTargetFlags |= ImGuiDragDropFlags_AcceptNoDrawDefaultRect; // Don't display the yellow rectangle

                if (const ImGuiPayload* _pPayload = ImGui::AcceptDragDropPayload("ASSETS_DRAGDROP", ImGuiTargetFlags))
                {
                    int a = 4;
                }

                ImGui::EndDragDropTarget();
            }
        }

        // -----------------------------------------------------------------------------

        const char* GetHeader()
        {
            return "Material";
        }
    };
} // namespace Dt
