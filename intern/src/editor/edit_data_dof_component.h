
#pragma once

#include "engine/data/data_component_manager.h"
#include "engine/data/data_dof_component.h"
#include "engine/data/data_entity_manager.h"

#include "editor/imgui/imgui.h"

namespace Dt
{
    class CDOFComponentGUI : public CDOFComponent
    {
    public:

        bool OnGUI()
        {
            bool HasChanged = false;

            HasChanged |= ImGui::DragFloat("Near Distance", &m_NearDistance, 0.01f);

            HasChanged |= ImGui::DragFloat("Far Distance", &m_FarDistance, 0.01f);

            HasChanged |= ImGui::DragFloat("Near to far ratio", &m_NearToFarRatio, 0.01f);

            HasChanged |= ImGui::DragFloat("Fade Un- to Small blur", &m_FadeUnToSmallBlur, 0.01f);

            HasChanged |= ImGui::DragFloat("Fade Small- to medium blur", &m_FadeSmallToMediumBlur, 0.01f);

            if (HasChanged) UpdateEffect();

            return HasChanged;
        }

        // -----------------------------------------------------------------------------

        void OnNewComponent(Dt::CEntity::BID _ID)
        {
            Dt::CEntity* pCurrentEntity = Dt::CEntityManager::GetInstance().GetEntityByID(_ID);

            pCurrentEntity->SetCategory(Dt::SEntityCategory::Dynamic);

            auto pComponent = Dt::CComponentManager::GetInstance().Allocate<Dt::CDOFComponent>();

            pCurrentEntity->AttachComponent(pComponent);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pComponent, Dt::CDOFComponent::DirtyCreate);
        }

        // -----------------------------------------------------------------------------

        void OnDropAsset(const Edit::CAsset&)
        {
        }
    };
} // namespace Dt
