
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
            ImGui::Checkbox("Enable Selection", &m_IsSelectionEnabled);
            ImGui::Checkbox("Enable Mouse Control", &m_IsMouseControlEnabled);
            
            ImGui::Checkbox("Play recording", &m_IsPlaying);

            /*if (m_IsPlayingRecording)
            {
                if (ImGui::Button("Pause"))
                {
                    m_IsPlayingRecording = false;
                }
            }
            else
            {
                if (ImGui::Button("Play"))
                {
                    m_IsPlayingRecording = true;
                }
            }*/

            if (ImGui::Button("Colorize Planes"))
            {
                ColorizePlanes();
            }
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
