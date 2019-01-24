
#pragma once

#include "engine/data/data_transformation_facet.h"

#include "imgui.h"

namespace Dt
{
    class CTransformationFacetGUI : public CTransformationFacet
    {
    public:
        
        void OnGUI()
        {
            ImGui::DragFloat3("Position", &m_Position.x, 0.001f);
            ImGui::DragFloat3("Rotation", &m_Rotation.x, 0.1f);
            ImGui::DragFloat3("Scale", &m_Scale.x, 0.001f);
        }

        // -----------------------------------------------------------------------------

        const char* GetHeader()
        {
            return "Transformation";
        }
    };
} // namespace Dt
