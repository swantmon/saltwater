
#pragma once

#include "engine/data/data_component_manager.h"
#include "engine/data/data_entity_manager.h"
#include "engine/data/data_light_probe_component.h"

#include "editor/imgui/imgui.h"

namespace Dt
{
    class CLightProbeComponentGUI : public CLightProbeComponent
    {
    public:

        bool OnGUI()
        {
            bool HasChanged = false;

            // -----------------------------------------------------------------------------
            // Refresh mode
            // -----------------------------------------------------------------------------
            {
                const char* Text[] = { "Static", "Dynamic" };

                auto Index = static_cast<int>(GetRefreshMode());

                HasChanged |= ImGui::Combo("Refresh Mode", &Index, Text, 2);

                SetRefreshMode(static_cast<ERefreshMode>(Index));
            }

            // -----------------------------------------------------------------------------
            // Type
            // -----------------------------------------------------------------------------
            {
                const char* Text[] = { "Sky", "Local", "Custom" };

                auto Index = static_cast<int>(GetType());

                HasChanged |= ImGui::Combo("Type", &Index, Text, 3);

                SetType(static_cast<EType>(Index));
            }

            // -----------------------------------------------------------------------------
            // Quality
            // -----------------------------------------------------------------------------
            {
                const char* Text[] = { "32", "64", "128", "256", "512", "1024", "2048" };

                auto Index = static_cast<int>(GetQuality());

                HasChanged |= ImGui::Combo("Quality", &Index, Text, 7);

                SetQuality(static_cast<EQuality>(Index));
            }

            // -----------------------------------------------------------------------------
            // Clear flag
            // -----------------------------------------------------------------------------
            {
                const char* Text[] = { "Skybox", "Black" };

                auto Index = static_cast<int>(GetClearFlag());

                HasChanged |= ImGui::Combo("Clear Flag", &Index, Text, 2);

                SetClearFlag(static_cast<EClearFlag>(Index));
            }

            // -----------------------------------------------------------------------------
            // Rest
            // -----------------------------------------------------------------------------
            HasChanged |= ImGui::DragFloat("Intensity", &m_Intensity);

            HasChanged |= ImGui::DragFloat("Near", &m_Near);

            HasChanged |= ImGui::DragFloat("Far", &m_Far);

            HasChanged |= ImGui::Checkbox("Parallax Correction", &m_ParallaxCorrection);

            HasChanged |= ImGui::DragFloat3("Box Size", &m_BoxSize.x);

            return HasChanged;
        }

        // -----------------------------------------------------------------------------

        const char* GetHeader()
        {
            return "Light Probe";
        }

        // -----------------------------------------------------------------------------

        void OnNewComponent(Dt::CEntity::BID _ID)
        {
            Dt::CEntity* pCurrentEntity = Dt::CEntityManager::GetInstance().GetEntityByID(_ID);

            pCurrentEntity->SetCategory(Dt::SEntityCategory::Dynamic);

            auto pComponent = Dt::CComponentManager::GetInstance().Allocate<Dt::CLightProbeComponent>();

            pCurrentEntity->AttachComponent(pComponent);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pComponent, Dt::CLightProbeComponent::DirtyCreate);
        }

        // -----------------------------------------------------------------------------

        void OnDropAsset(const Edit::CAsset&)
        {
        }
    };
} // namespace Dt
