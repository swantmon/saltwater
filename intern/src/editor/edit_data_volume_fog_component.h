
#pragma once

#include "engine/data/data_component_manager.h"
#include "engine/data/data_entity_manager.h"
#include "engine/data/data_volume_fog_component.h"

#include "editor/imgui/imgui.h"

namespace Dt
{
    class CVolumeFogComponentGUI : public CVolumeFogComponent
    {
    public:

        void OnGUI()
        {
            ImGui::DragFloat3("Wind Direction", &m_WindDirection.x);

            ImGui::DragFloat("Wind Speed", &m_WindDirection.w);

            ImGui::DragFloat("Frustum Depth", &m_FrustumDepthInMeter, 1.0f, 0.0f, 0.0f, "%.3f units");

            ImGui::DragFloat("Shadow Intensity", &m_ShadowIntensity);

            ImGui::DragFloat("Scattering Coefficient", &m_ScatteringCoefficient);

            ImGui::DragFloat("Absorption Coefficient", &m_AbsorptionCoefficient);

            ImGui::DragFloat("Density Level", &m_DensityLevel);

            ImGui::DragFloat("Density Attenuation", &m_DensityAttenuation);
        }

        // -----------------------------------------------------------------------------

        const char* GetHeader()
        {
            return "Volumetric Fog";
        }

        // -----------------------------------------------------------------------------

        void OnNewComponent(Dt::CEntity::BID _ID)
        {
            Dt::CEntity* pCurrentEntity = Dt::CEntityManager::GetInstance().GetEntityByID(_ID);

            pCurrentEntity->SetCategory(Dt::SEntityCategory::Dynamic);

            auto pComponent = Dt::CComponentManager::GetInstance().Allocate<Dt::CVolumeFogComponent>();

            pCurrentEntity->AttachComponent(pComponent);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pComponent, Dt::CVolumeFogComponent::DirtyCreate);
        }

        // -----------------------------------------------------------------------------

        void OnDropAsset(const Edit::CAsset&)
        {
        }
    };
} // namespace Dt
