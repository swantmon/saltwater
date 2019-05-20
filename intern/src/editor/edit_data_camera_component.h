
#pragma once

#include "engine/data/data_camera_component.h"
#include "engine/data/data_component_manager.h"
#include "engine/data/data_entity_manager.h"

#include "editor/imgui/imgui.h"

namespace Dt
{
    class CCameraComponentGUI : public CCameraComponent
    {
    public:

        void OnGUI()
        {
            // -----------------------------------------------------------------------------
            // Clear flag
            // -----------------------------------------------------------------------------
            {
                const char* Text[] = { "Skybox", "Webcam", "Solid Color", "No clear" };

                int Index = static_cast<int>(GetClearFlag());

                ImGui::Combo("Clear Flag", &Index, Text, 4);

                SetClearFlag(static_cast<EClearFlag>(Index));
            }

            if (GetClearFlag() == EClearFlag::SolidColor)
            {
                ImGui::ColorEdit3("Solid Color", &m_BackgroundColor.r);
            }

            // -----------------------------------------------------------------------------
            // Projection
            // -----------------------------------------------------------------------------
            {
                const char* Text[] = { "Perspective", "Orthographic" };

                int Index = static_cast<int>(GetProjectionType());

                ImGui::Combo("Projection", &Index, Text, 2);

                SetProjectionType(static_cast<EProjectionType>(Index));
            }

            if (GetProjectionType() == CCameraComponent::Perspective)
            {
                ImGui::SliderFloat("Field of View", &m_FoV, 0.0f, 179.0f, "%.0f");
            }
            else if (GetProjectionType() == CCameraComponent::Orthographic)
            {
                ImGui::DragFloat("Size", &m_Size);
            }

            // -----------------------------------------------------------------------------
            // Clipping planes
            // -----------------------------------------------------------------------------
            ImGui::TextDisabled("Clipping Planes");

            ImGui::DragFloat("Near", &m_Near);

            ImGui::DragFloat("Far", &m_Far);

            // -----------------------------------------------------------------------------
            // Depth
            // -----------------------------------------------------------------------------
            ImGui::DragFloat("Depth", &m_Depth);

            // -----------------------------------------------------------------------------
            // Camera mode
            // -----------------------------------------------------------------------------
            {
                const char* Text[] = { "Auto", "Manual" };

                int Index = static_cast<int>(GetCameraMode());

                ImGui::Combo("Camera Mode", &Index, Text, 2);

                SetCameraMode(static_cast<ECameraMode>(Index));
            }

            if (GetCameraMode() == CCameraComponent::Manual)
            {
                ImGui::DragFloat("Shutter Speed", &m_ShutterSpeed, 0.01f, 0.0f, 0.0f, "%.2f seconds");

                ImGui::DragFloat("Aperture", &m_Aperture, 0.01f, 0.0f, 0.0f, "%.2f flops");

                ImGui::DragFloat("ISO", &m_ISO, 1.0f, 0.0f, 0.0f, "%.0f");

                ImGui::DragFloat("Exposure", &m_EC, 0.1f, -5.0f, 5.0f, "%.1f");
            }

            // -----------------------------------------------------------------------------
            // Viewport Rect
            // -----------------------------------------------------------------------------
            ImGui::TextDisabled("Viewport");

            ImGui::DragFloat2("XY", &m_ViewportRect.x);

            ImGui::DragFloat2("WH", &m_ViewportRect.z);
        }

        // -----------------------------------------------------------------------------

        const char* GetHeader()
        {
            return "Camera";
        }

        // -----------------------------------------------------------------------------

        void OnNewComponent(Dt::CEntity::BID _ID)
        {
            Dt::CEntity* pCurrentEntity = Dt::CEntityManager::GetInstance().GetEntityByID(_ID);

            pCurrentEntity->SetCategory(Dt::SEntityCategory::Dynamic);

            auto pComponent = Dt::CComponentManager::GetInstance().Allocate<Dt::CCameraComponent>();

            pCurrentEntity->AttachComponent(pComponent);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pComponent, Dt::CCameraComponent::DirtyCreate);
        }

        // -----------------------------------------------------------------------------

        void OnDropAsset(const Edit::CAsset&)
        {
        }
    };
} // namespace Dt
