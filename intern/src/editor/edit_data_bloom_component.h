
#pragma once

#include "engine/data/data_bloom_component.h"
#include "engine/data/data_component_manager.h"
#include "engine/data/data_entity_manager.h"

#include "editor/imgui/imgui.h"

namespace Dt
{
    class CBloomComponentGUI : public CBloomComponent
    {
    public:

        bool OnGUI()
        {
            bool HasChanged = false;

            HasChanged |= ImGui::ColorEdit4("Tint", &m_Tint.x);

            HasChanged |= ImGui::DragFloat("Intensity", &m_Intensity, 0.1f, 0.0f, 8.0f);

            HasChanged |= ImGui::DragFloat("Threshold", &m_Treshhold, 0.1f, -1.0f, 8.0f);

            HasChanged |= ImGui::DragFloat("Exposure Scale", &m_ExposureScale, 0.1f);

            unsigned int Min = 0;
            unsigned int Max = 5;

            HasChanged |= ImGui::DragScalar("Scale", ImGuiDataType_U32, &m_Size, 1.0f, &Min, &Max);

            if (HasChanged) UpdateEffect();

            return HasChanged;
        }

        // -----------------------------------------------------------------------------

        void OnNewComponent(Dt::CEntity::BID _ID)
        {
            Dt::CEntity* pCurrentEntity = Dt::CEntityManager::GetInstance().GetEntityByID(_ID);

            pCurrentEntity->SetCategory(Dt::SEntityCategory::Dynamic);

            auto pComponent = Dt::CComponentManager::GetInstance().Allocate<Dt::CBloomComponent>();

            pCurrentEntity->AttachComponent(pComponent);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pComponent, Dt::CBloomComponent::DirtyCreate);
        }

        // -----------------------------------------------------------------------------

        void OnDropAsset(const Edit::CAsset&)
        {
        }
    };
} // namespace Dt
