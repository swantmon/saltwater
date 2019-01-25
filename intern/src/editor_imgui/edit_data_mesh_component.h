
#pragma once

#include "engine/data/data_mesh_component.h"

#include "editor_imgui/imgui/imgui.h"

namespace Dt
{
    class CMeshComponentGUI : public CMeshComponent
    {
    public:

        void OnGUI()
        {
            {
                const char* Text[] = { "Asset", "Box", "Sphere", "IsometricSphere", "Cone", "Rectangle" };

                int Index = static_cast<int>(GetMeshType());

                ImGui::LabelText("Mesh Type", Text[Index]);
            }

            if (GetMeshType() == CMeshComponent::Asset)
            {
                ImGui::LabelText("File", m_Filename.c_str());
            }
        }

        // -----------------------------------------------------------------------------

        const char* GetHeader()
        {
            return "Mesh";
        }
    };
} // namespace Dt
