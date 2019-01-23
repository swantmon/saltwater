
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
            ImGui::Text("Transformation Facet");
        }
    };
} // namespace Dt
