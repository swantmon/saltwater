
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

        void OnGUI()
        {
            ImGui::DragFloat("Intensity", &m_Intensity);

            ImGui::DragFloat("Roughness", &m_RoughnessMask);

            ImGui::DragFloat("Distance", &m_Distance);

            ImGui::Checkbox("Use last frame", &m_UseLastFrame);
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
