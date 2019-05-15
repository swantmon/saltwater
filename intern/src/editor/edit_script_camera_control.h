
#pragma once

#include "engine/data/data_component_manager.h"
#include "engine/data/data_entity_manager.h"

#include "engine/script/script_camera_control_script.h"

#include "editor/imgui/imgui.h"

namespace Scpt
{
    class CCameraControlScriptGUI : public Scpt::CCameraControlScript
    {
    public:

        void OnGUI()
        {
            ImGui::Text("Not implemented...");
        }

        // -----------------------------------------------------------------------------

        const char* GetHeader()
        {
            return "Camera Control (Script)";
        }

        // -----------------------------------------------------------------------------

        void OnNewComponent(Dt::CEntity::BID _ID)
        {
            Dt::CEntity* pCurrentEntity = Dt::EntityManager::GetEntityByID(_ID);

            pCurrentEntity->SetCategory(Dt::SEntityCategory::Dynamic);

            auto pComponent = Dt::CComponentManager::GetInstance().Allocate<Scpt::CCameraControlScript>();

            pCurrentEntity->AttachComponent(pComponent);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pComponent, Scpt::CCameraControlScript::DirtyCreate);
        }

        // -----------------------------------------------------------------------------

        void OnDropAsset(const Edit::CAsset&)
        {
        }
    };
} // namespace Scpt
