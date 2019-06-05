
#pragma once

#include "engine/data/data_mesh_component.h"

#include "editor/imgui/imgui.h"

namespace Dt
{
    class CMeshComponentGUI : public CMeshComponent
    {
    public:

        bool OnGUI()
        {
            const char* Text[] = { "Asset", "Box", "Sphere", "IsometricSphere", "Cone", "Rectangle" };

            auto Index = static_cast<int>(GetMeshType());

            ImGui::LabelText("Mesh Type", Text[Index]);

            if (GetMeshType() == CMeshComponent::Asset)
            {
                ImGui::LabelText("File", m_Filename.c_str());
            }

            return false;
        }

        // -----------------------------------------------------------------------------

        const char* GetHeader()
        {
            return "Mesh";
        }

        // -----------------------------------------------------------------------------

        void OnDropAsset(const Edit::CAsset&)
        {
        }
    };
} // namespace Dt
