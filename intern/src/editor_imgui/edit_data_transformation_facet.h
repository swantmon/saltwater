
#pragma once

#include "engine/data/data_transformation_facet.h"

#include "editor_imgui/imgui/imgui.h"

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
            glm::vec3 RotationDegree;

            RotationDegree.x = glm::degrees(m_Rotation.x);
            RotationDegree.y = glm::degrees(m_Rotation.y);
            RotationDegree.z = glm::degrees(m_Rotation.z);

            ImGui::DragFloat3("Rotation", &RotationDegree.x, 0.1f, 0.0f, 0.0f, "%.2f");

            m_Rotation.x = glm::radians(RotationDegree.x);
            m_Rotation.y = glm::radians(RotationDegree.y);
            m_Rotation.z = glm::radians(RotationDegree.z);

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
