
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

        void OnGUI()
        {
            const char* RefreshModeText[] = { "Static", "Dynamic" };

            int RefreshModeIndex = static_cast<int>(GetRefreshMode());

            ImGui::Combo("Refresh Mode", &RefreshModeIndex, RefreshModeText, 2);

            SetRefreshMode(static_cast<ERefreshMode>(RefreshModeIndex));

            ImGui::ColorEdit3("Color", &m_Color.r);

            ImGui::DragFloat("Intensity", &m_Intensity);

            ImGui::DragFloat3("Direction", &m_Direction.x, 0.001f);

            UpdateLightness();
        }

        // -----------------------------------------------------------------------------

        const char* GetHeader()
        {
            return "Sun";
        }

        // -----------------------------------------------------------------------------

        void OnNewComponent(Dt::CEntity::BID _ID)
        {
            Dt::CEntity* pCurrentEntity = Dt::EntityManager::GetEntityByID(_ID);

            pCurrentEntity->SetCategory(Dt::SEntityCategory::Dynamic);

            auto pComponent = Dt::CComponentManager::GetInstance().Allocate<Dt::CSunComponent>();

            pCurrentEntity->AttachComponent(pComponent);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pComponent, Dt::CSunComponent::DirtyCreate);
        }
    };
} // namespace Dt
