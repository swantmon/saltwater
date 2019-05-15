
#pragma once

#include "base/base_include_glm.h"

#include "editor/edit_edit_state.h"
#include "editor/imgui/imgui.h"
#include "editor/imgui/extensions/ImGuizmo.h"

#include "engine/core/core_console.h"

#include "engine/data/data_transformation_facet.h"

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
            auto Operation = Edit::CEditState::GetInstance().GetOperation();
            auto Mode      = Edit::CEditState::GetInstance().GetMode();

            static ImGuizmo::OPERATION CurrentGizmoOperation(ImGuizmo::TRANSLATE);
            static ImGuizmo::MODE CurrentGizmoMode(ImGuizmo::WORLD);

            switch (Operation)
            {
            case Edit::CEditState::Translate:
                CurrentGizmoOperation = ImGuizmo::TRANSLATE;
                break;
            case Edit::CEditState::Rotate:
                CurrentGizmoOperation = ImGuizmo::ROTATE;
                break;
            case Edit::CEditState::Scale:
                CurrentGizmoOperation = ImGuizmo::SCALE;
                break;
            case Edit::CEditState::Hand:
            default:
                break;
            }

            switch (Mode)
            {
            case Edit::CEditState::Local:
                CurrentGizmoMode = ImGuizmo::LOCAL;
                break;
            case Edit::CEditState::World:
            default:
                CurrentGizmoMode = ImGuizmo::WORLD;
                break;
            }
            if (Operation != Edit::CEditState::Hand)
            {
                glm::mat4 WorldMatrix;

                ImGuizmo::RecomposeMatrixFromComponents(&m_Position.x, &DegreeAngles.x, &m_Scale.x, &WorldMatrix[0][0]);

                ImGuiIO& io = ImGui::GetIO();
                ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
                ImGuizmo::Manipulate(&ViewMatrix[0][0], &ProjMatrix[0][0], CurrentGizmoOperation, CurrentGizmoMode, &WorldMatrix[0][0], nullptr, nullptr);

                ImGuizmo::DecomposeMatrixToComponents(&WorldMatrix[0][0], &m_Position.x, &DegreeAngles.x, &m_Scale.x);
            }

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

        // -----------------------------------------------------------------------------

        void OnDropAsset(const Edit::CAsset&)
        {
        }
    };
} // namespace Dt
