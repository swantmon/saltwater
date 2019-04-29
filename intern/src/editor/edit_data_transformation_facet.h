
#pragma once

#include "base/base_include_glm.h"

#include "engine/data/data_transformation_facet.h"

#include "editor/imgui/imgui.h"

namespace Dt
{
    class CTransformationFacetGUI : public CTransformationFacet
    {
    public:
        
        void OnGUI()
        {
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

			EulerAngles.x = glm::radians(DegreeAngles.x);
			EulerAngles.y = glm::radians(DegreeAngles.y);
			EulerAngles.z = glm::radians(DegreeAngles.z);

			m_Rotation = glm::quat(EulerAngles);

            // -----------------------------------------------------------------------------
            // Scale
            // -----------------------------------------------------------------------------
            ImGui::DragFloat3("Scale", &m_Scale.x, 0.01f);
        }

        // -----------------------------------------------------------------------------

        const char* GetHeader()
        {
            return "Transformation";
        }
    };
} // namespace Dt
