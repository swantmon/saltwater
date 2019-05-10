
#pragma once

#include "base/base_include_glm.h"

#include "engine/core/core_console.h"

#include "engine/data/data_transformation_facet.h"

#include "editor/imgui/imgui.h"
#include "editor/imgui/extensions/ImGuizmo.h"

#include "engine/graphic/gfx_view_manager.h"

namespace Dt
{
    class CTransformationFacetGUI : public CTransformationFacet
    {
    public:
        
        void OnGUI()
        {
            auto Camera = Gfx::ViewManager::GetMainCamera();

            auto ViewMatrix = Camera->GetView()->GetViewMatrix();
            auto ProjMatrix = Camera->GetProjectionMatrix();

            // -----------------------------------------------------------------------------
            // GUIZMO
            // -----------------------------------------------------------------------------
            static ImGuizmo::OPERATION CurrentGizmoOperation(ImGuizmo::ROTATE);
            static ImGuizmo::MODE CurrentGizmoMode(ImGuizmo::WORLD);

            if (ImGui::RadioButton("Translate", CurrentGizmoOperation == ImGuizmo::TRANSLATE))
            {
                CurrentGizmoOperation = ImGuizmo::TRANSLATE;
            }

            ImGui::SameLine();

            if (ImGui::RadioButton("Rotate", CurrentGizmoOperation == ImGuizmo::ROTATE))
            {
                CurrentGizmoOperation = ImGuizmo::ROTATE;
            }

            ImGui::SameLine();

            if (ImGui::RadioButton("Scale", CurrentGizmoOperation == ImGuizmo::SCALE))
            {
                CurrentGizmoOperation = ImGuizmo::SCALE;
            }

            if (CurrentGizmoOperation != ImGuizmo::SCALE)
            {
                if (ImGui::RadioButton("Local", CurrentGizmoMode == ImGuizmo::LOCAL))
                {
                    CurrentGizmoMode = ImGuizmo::LOCAL;
                }

                ImGui::SameLine();

                if (ImGui::RadioButton("World", CurrentGizmoMode == ImGuizmo::WORLD))
                {
                    CurrentGizmoMode = ImGuizmo::WORLD;
                }
            }

            // -----------------------------------------------------------------------------
            // Position
            // -----------------------------------------------------------------------------
            ImGui::DragFloat3("Position", &m_Position.x, 0.01f);

            // -----------------------------------------------------------------------------
            // Rotation
            // -----------------------------------------------------------------------------
			glm::vec3 EulerAngles;
            glm::vec3 DegreeAngles;

			EulerAngles = glm::eulerAngles(m_Rotation);

            DegreeAngles.x = glm::degrees(EulerAngles.x);
            DegreeAngles.y = glm::degrees(EulerAngles.y);
            DegreeAngles.z = glm::degrees(EulerAngles.z);

            ImGui::DragFloat3("Rotation", &DegreeAngles.x, 0.1f, 0.0f, 0.0f, "%.2f");

            // -----------------------------------------------------------------------------
            // Scale
            // -----------------------------------------------------------------------------
            ImGui::DragFloat3("Scale", &m_Scale.x, 0.01f);

            // -----------------------------------------------------------------------------
            // GUIZMO
            // -----------------------------------------------------------------------------
            glm::mat4 WorldMatrix;

            ImGuizmo::RecomposeMatrixFromComponents(&m_Position.x, &DegreeAngles.x, &m_Scale.x, &WorldMatrix[0][0]);

            ImGuiIO& io = ImGui::GetIO();
            ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
            ImGuizmo::Manipulate(&ViewMatrix[0][0], &ProjMatrix[0][0], CurrentGizmoOperation, CurrentGizmoMode, &WorldMatrix[0][0], nullptr, nullptr);

            ImGuizmo::DecomposeMatrixToComponents(&WorldMatrix[0][0], &m_Position.x, &DegreeAngles.x, &m_Scale.x);

            // -----------------------------------------------------------------------------
            // Apply rotation
            // -----------------------------------------------------------------------------
            EulerAngles.x = glm::radians(DegreeAngles.x);
            EulerAngles.y = glm::radians(DegreeAngles.y);
            EulerAngles.z = glm::radians(DegreeAngles.z);

            m_Rotation = glm::quat(EulerAngles);
        }

        // -----------------------------------------------------------------------------

        const char* GetHeader()
        {
            return "Transformation";
        }
    };
} // namespace Dt
