
#pragma once

#include "engine/data/data_component_manager.h"
#include "engine/data/data_entity_manager.h"

#include "engine/script/script_slam.h"

#include "editor/imgui/imgui.h"

namespace Scpt
{
    class CSLAMScriptGUI : public Scpt::CSLAMScript
    {
    public:

        void OnGUI()
        {
            ImGui::Checkbox("Enable Selection", &m_Settings.m_IsMouseControlEnabled);
            ImGui::Checkbox("Enable Mouse Control", &m_Settings.m_IsMouseControlEnabled);
            ImGui::Checkbox("Permanent Colorization", &m_Settings.m_IsPermanentColorizationEnabled);
            ImGui::Checkbox("Play Recording", &m_Settings.m_IsPlayingRecording);

            m_Settings.m_Colorize = (ImGui::Button("Colorize Planes"));
        }

        // -----------------------------------------------------------------------------

        const char* GetHeader()
        {
            return "SLAM (Script)";
        }

        // -----------------------------------------------------------------------------

        void OnNewComponent(Dt::CEntity::BID _ID)
        {
            Dt::CEntity* pCurrentEntity = Dt::EntityManager::GetEntityByID(_ID);

            pCurrentEntity->SetCategory(Dt::SEntityCategory::Dynamic);

            auto pComponent = Dt::CComponentManager::GetInstance().Allocate<Scpt::CSLAMScriptGUI>();

            pCurrentEntity->AttachComponent(pComponent);

            Dt::CComponentManager::GetInstance().MarkComponentAsDirty(*pComponent, Scpt::CSLAMScriptGUI::DirtyCreate);
        }

        // -----------------------------------------------------------------------------

        void OnDropAsset(const Edit::CAsset&)
        {
        }
    };
} // namespace Scpt
