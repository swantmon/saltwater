
#pragma once

#include "engine/data/data_component_manager.h"
#include "engine/data/data_entity_manager.h"
#include "engine/data/data_sun_component.h"

#include "editor/imgui/imgui.h"

namespace Dt
{
    class CSunComponentGUI : public CSunComponent
    {
    public:

        bool OnGUI()
        {
            bool HasChanged = false;

            const char* RefreshModeText[] = { "Static", "Dynamic" };

            auto RefreshModeIndex = static_cast<int>(GetRefreshMode());

            HasChanged |= ImGui::Combo("Refresh Mode", &RefreshModeIndex, RefreshModeText, 2);

            SetRefreshMode(static_cast<ERefreshMode>(RefreshModeIndex));

            HasChanged |= ImGui::ColorEdit3("Color", &m_Color.r);

            HasChanged |= ImGui::DragFloat("Intensity", &m_Intensity);

            HasChanged |= ImGui::DragFloat("Distance From Origin", &m_CustomDistanceFromOrigin);

            HasChanged |= ImGui::DragFloat3("Direction", &m_Direction.x, 0.001f);

            if (HasChanged) UpdateLightness();

            return HasChanged;
        }

        // -----------------------------------------------------------------------------

        static void OnNewComponent(Dt::CEntity::BID _ID)
        {
            Dt::CEntity* pCurrentEntity = Dt::CEntityManager::GetInstance().GetEntityByID(_ID);

            pCurrentEntity->SetCategory(Dt::SEntityCategory::Dynamic);

            auto pComponent = Dt::CComponentManager::GetInstance().Allocate<Dt::CSunComponent>();

            pCurrentEntity->AttachComponent(pComponent);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pComponent, Dt::CSunComponent::DirtyCreate);
        }

        // -----------------------------------------------------------------------------

        void OnDropAsset(const Edit::CAsset&)
        {
        }
    };
} // namespace Dt
