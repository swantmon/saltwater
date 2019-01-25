
#pragma once

#include "engine/data/data_component_manager.h"
#include "engine/data/data_entity_manager.h"
#include "engine/data/data_post_aa_component.h"

#include "editor_imgui/imgui/imgui.h"

namespace Dt
{
    class CPostAAComponentGUI : public CPostAAComponent
    {
    public:

        void OnGUI()
        {
            // -----------------------------------------------------------------------------
            // Type
            // -----------------------------------------------------------------------------
            {
                const char* Text[EType::NumberOfTypes] = { "SMAA", "FXAA" };

                int Index = static_cast<int>(GetType());

                ImGui::Combo("Type", &Index, Text, 2);

                SetType(static_cast<EType>(Index));
            }
        }

        // -----------------------------------------------------------------------------

        const char* GetHeader()
        {
            return "Anti Aliasing";
        }

        // -----------------------------------------------------------------------------

        void OnNewComponent(Dt::CEntity::BID _ID)
        {
            Dt::CEntity* pCurrentEntity = Dt::EntityManager::GetEntityByID(_ID);

            pCurrentEntity->SetCategory(Dt::SEntityCategory::Dynamic);

            auto pComponent = Dt::CComponentManager::GetInstance().Allocate<Dt::CPostAAComponent>();

            pCurrentEntity->AttachComponent(pComponent);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pComponent, Dt::CPostAAComponent::DirtyCreate);
        }
    };
} // namespace Dt
