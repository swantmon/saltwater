
#pragma once

#include "engine/data/data_component_manager.h"
#include "engine/data/data_entity_manager.h"
#include "engine/data/data_ssr_component.h"

#include "editor/imgui/imgui.h"

namespace Dt
{
    class CSSRComponentGUI : public CSSRComponent
    {
    public:

        bool OnGUI()
        {
            bool HasChanged = false;

            HasChanged |= ImGui::DragFloat("Intensity", &m_Intensity);

            HasChanged |= ImGui::DragFloat("Roughness", &m_RoughnessMask);

            HasChanged |= ImGui::DragFloat("Distance", &m_Distance);

            HasChanged |= ImGui::Checkbox("Use last frame", &m_UseLastFrame);

            return HasChanged;
        }

        // -----------------------------------------------------------------------------

        const char* GetHeader()
        {
            return "Screen Space Reflections";
        }

        // -----------------------------------------------------------------------------

        void OnNewComponent(Dt::CEntity::BID _ID)
        {
            Dt::CEntity* pCurrentEntity = Dt::CEntityManager::GetInstance().GetEntityByID(_ID);

            pCurrentEntity->SetCategory(Dt::SEntityCategory::Dynamic);

            auto pComponent = Dt::CComponentManager::GetInstance().Allocate<Dt::CSSRComponent>();

            pCurrentEntity->AttachComponent(pComponent);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pComponent, Dt::CSSRComponent::DirtyCreate);
        }

        // -----------------------------------------------------------------------------

        void OnDropAsset(const Edit::CAsset&)
        {
        }
    };
} // namespace Dt
