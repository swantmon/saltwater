
#pragma once

#include "engine/data/data_component_manager.h"
#include "engine/data/data_entity_manager.h"
#include "engine/data/data_ssao_component.h"

#include "editor/imgui/imgui.h"

namespace Dt
{
    class CSSAOComponentGUI : public CSSAOComponent
    {
    public:

        void OnGUI()
        {
            
        }

        // -----------------------------------------------------------------------------

        const char* GetHeader()
        {
            return "Ambient Occlusion";
        }

        // -----------------------------------------------------------------------------

        void OnNewComponent(Dt::CEntity::BID _ID)
        {
            Dt::CEntity* pCurrentEntity = Dt::EntityManager::GetEntityByID(_ID);

            pCurrentEntity->SetCategory(Dt::SEntityCategory::Dynamic);

            auto pComponent = Dt::CComponentManager::GetInstance().Allocate<Dt::CSSAOComponent>();

            pCurrentEntity->AttachComponent(pComponent);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pComponent, Dt::CSSAOComponent::DirtyCreate);
        }
    };
} // namespace Dt
