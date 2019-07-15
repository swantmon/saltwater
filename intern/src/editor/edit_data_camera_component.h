
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

        bool OnGUI()
        {
            bool HasChanged = false;

            // -----------------------------------------------------------------------------
            // Clear flag
            // -----------------------------------------------------------------------------
            {
                const char* Text[] = { "Skybox", "Webcam", "Solid Color", "No clear" };

                auto Index = static_cast<int>(GetClearFlag());

                if (ImGui::Combo("Clear Flag", &Index, Text, 4))
                {
                    SetClearFlag(static_cast<EClearFlag>(Index));

                    HasChanged = true;
                }
            }

            if (GetClearFlag() == EClearFlag::SolidColor)
            {
                HasChanged |= ImGui::ColorEdit3("Solid Color", &m_BackgroundColor.r);
            }

            // -----------------------------------------------------------------------------
            // Projection
            // -----------------------------------------------------------------------------
            {
                const char* Text[] = { "Perspective", "Orthographic" };

                auto Index = static_cast<int>(GetProjectionType());

                if (ImGui::Combo("Projection", &Index, Text, 2))
                {
                    SetProjectionType(static_cast<EProjectionType>(Index));

                    HasChanged = true;
                }
            }

            if (GetProjectionType() == CCameraComponent::Perspective)
            {
                HasChanged |= ImGui::SliderFloat("Field of View", &m_FoV, 0.0f, 179.0f, "%.0f");
            }
            else if (GetProjectionType() == CCameraComponent::Orthographic)
            {
                HasChanged |= ImGui::DragFloat("Size", &m_Size);
            }

            // -----------------------------------------------------------------------------
            // Clipping planes
            // -----------------------------------------------------------------------------
            ImGui::TextDisabled("Clipping Planes");

            HasChanged |= ImGui::DragFloat("Near", &m_Near);

            HasChanged |= ImGui::DragFloat("Far", &m_Far);

            // -----------------------------------------------------------------------------
            // Depth
            // -----------------------------------------------------------------------------
            HasChanged |= ImGui::DragFloat("Depth", &m_Depth);

            // -----------------------------------------------------------------------------
            // Camera mode
            // -----------------------------------------------------------------------------
            {
                const char* Text[] = { "Auto", "Manual" };

                auto Index = static_cast<int>(GetCameraMode());

                if (ImGui::Combo("Camera Mode", &Index, Text, 2))
                {
                    SetCameraMode(static_cast<ECameraMode>(Index));

                    HasChanged = true;
                }
                
            }

            if (GetCameraMode() == CCameraComponent::Manual)
            {
                HasChanged |= ImGui::DragFloat("Shutter Speed", &m_ShutterSpeed, 0.01f, 0.0f, 0.0f, "%.2f seconds");

                HasChanged |= ImGui::DragFloat("Aperture", &m_Aperture, 0.01f, 0.0f, 0.0f, "%.2f flops");

                HasChanged |= ImGui::DragFloat("ISO", &m_ISO, 1.0f, 0.0f, 0.0f, "%.0f");

                HasChanged |= ImGui::DragFloat("Exposure", &m_EC, 0.1f, -5.0f, 5.0f, "%.1f");
            }

            // -----------------------------------------------------------------------------
            // Viewport Rect
            // -----------------------------------------------------------------------------
            ImGui::TextDisabled("Viewport");

            HasChanged |= ImGui::DragFloat2("XY", &m_ViewportRect.x);

            HasChanged |= ImGui::DragFloat2("WH", &m_ViewportRect.z);

            return HasChanged;
        }

        // -----------------------------------------------------------------------------

        static void OnNewComponent(Dt::CEntity::BID _ID)
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
