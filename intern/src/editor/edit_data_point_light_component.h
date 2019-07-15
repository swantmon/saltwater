
#pragma once

#include "engine/data/data_component_manager.h"
#include "engine/data/data_entity_manager.h"
#include "engine/data/data_point_light_component.h"

#include "editor/imgui/imgui.h"

namespace Dt
{
    class CPointLightComponentGUI : public CPointLightComponent
    {
    public:

        bool OnGUI()
        {
            bool HasChanged = false;

            HasChanged |= ImGui::ColorEdit3("Color", &m_Color.r);

            HasChanged |= ImGui::DragFloat("Intensity", &m_Intensity);

            HasChanged |= ImGui::DragFloat3("Direction", &m_Direction.x);

            HasChanged |= ImGui::DragFloat("Attenuation Radius", &m_AttentuationRadius, 0.01f, 0.0f, 100.0f, "%.2f");

            HasChanged |= ImGui::SliderAngle("Inner Cone", &m_InnerConeAngle, 0.0f, 180.0f, "%.0f angle");

            HasChanged |= ImGui::SliderAngle("Outer Cone", &m_OuterConeAngle, 0.0f, 180.0f, "%.0f angle");

            // -----------------------------------------------------------------------------
            // Shadow Type
            // -----------------------------------------------------------------------------
            {
                const char* Text[] = { "No Shadows", "Hard Shadows", "Global Illumination" };

                auto Index = static_cast<int>(GetShadowType());

                HasChanged |= ImGui::Combo("Shadow Type", &Index, Text, 3);

                SetShadowType(static_cast<EShadowType>(Index));
            }

            // -----------------------------------------------------------------------------
            // Shadow Quality
            // -----------------------------------------------------------------------------
            {
                const char* Text[] = { "Low", "Medium", "High", "Very High" };

                auto Index = static_cast<int>(GetShadowQuality());

                HasChanged |= ImGui::Combo("Shadow Quality", &Index, Text, 4);

                SetShadowQuality(static_cast<EShadowQuality>(Index));
            }

            // -----------------------------------------------------------------------------
            // Shadow Refresh
            // -----------------------------------------------------------------------------
            {
                const char* Text[] = { "Static", "Dynamic" };

                auto Index = static_cast<int>(GetRefreshMode());

                HasChanged |= ImGui::Combo("Shadow Refresh", &Index, Text, 2);

                SetRefreshMode(static_cast<ERefreshMode>(Index));
            }

            if (HasChanged) UpdateLightness();

            return HasChanged;
        }

        // -----------------------------------------------------------------------------

        static void OnNewComponent(Dt::CEntity::BID _ID)
        {
            Dt::CEntity* pCurrentEntity = Dt::CEntityManager::GetInstance().GetEntityByID(_ID);

            pCurrentEntity->SetCategory(Dt::SEntityCategory::Dynamic);

            auto pComponent = Dt::CComponentManager::GetInstance().Allocate<Dt::CPointLightComponent>();

            pCurrentEntity->AttachComponent(pComponent);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pComponent, Dt::CPointLightComponent::DirtyCreate);
        }

        // -----------------------------------------------------------------------------

        void OnDropAsset(const Edit::CAsset&)
        {
        }
    };
} // namespace Dt
