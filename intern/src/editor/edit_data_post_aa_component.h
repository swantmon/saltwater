
#pragma once

#include "engine/data/data_component_manager.h"
#include "engine/data/data_entity_manager.h"
#include "engine/data/data_post_aa_component.h"

#include "editor/imgui/imgui.h"

namespace Dt
{
    class CPostAAComponentGUI : public CPostAAComponent
    {
    public:

        bool OnGUI()
        {
            bool HasChanged = false;

            const char* Text[EType::NumberOfTypes] = { "SMAA", "FXAA" };

            auto Index = static_cast<int>(GetType());

            if (ImGui::Combo("Type", &Index, Text, 2))
            {
                SetType(static_cast<EType>(Index));

                HasChanged = true;
            }

            return HasChanged;
        }

        // -----------------------------------------------------------------------------

        static void OnNewComponent(Dt::CEntity::BID _ID)
        {
            Dt::CEntity* pCurrentEntity = Dt::CEntityManager::GetInstance().GetEntityByID(_ID);

            pCurrentEntity->SetCategory(Dt::SEntityCategory::Dynamic);

            auto pComponent = Dt::CComponentManager::GetInstance().Allocate<Dt::CPostAAComponent>();

            pCurrentEntity->AttachComponent(pComponent);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pComponent, Dt::CPostAAComponent::DirtyCreate);
        }

        // -----------------------------------------------------------------------------

        void OnDropAsset(const Edit::CAsset&)
        {
        }
    };
} // namespace Dt
