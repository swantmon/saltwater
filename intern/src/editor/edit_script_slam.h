
#pragma once

#include "editor/imgui/extensions/ImFiledialog.h"

#include "engine/core/core_asset_manager.h"
#include "engine/data/data_component_manager.h"
#include "engine/data/data_entity_manager.h"

#include "engine/script/script_slam.h"

#include "editor/imgui/imgui.h"

namespace Scpt
{
    class CSLAMScriptGUI : public Scpt::CSLAMScript
    {
    public:

        bool OnGUI()
        {
            ImGui::Checkbox("Enable Selection", &m_Settings.m_IsSelectionEnabled);
            ImGui::Checkbox("Enable Mouse Control", &m_Settings.m_IsMouseControlEnabled);
			ImGui::Checkbox("Permanent Colorization", &m_Settings.m_IsPermanentColorizationEnabled);
			m_Settings.m_SendPlanes = ImGui::Button("Send Planes");
            m_Settings.m_Colorize = ImGui::Button("Colorize Planes");

            const int ItemCount = 5;
            const char* pItems[ItemCount] = { "None", "Extent Only", "Mesh Only", "Mesh with Extent", "Mesh and Extent" };
            static const char* pCurrentItem = pItems[2];

            if (ImGui::BeginCombo("Plane Mode", pCurrentItem))
            {
                for (int i = 0; i < ItemCount; ++i)
                {
                    if (ImGui::Selectable(pItems[i]))
                    {
                        pCurrentItem = pItems[i];
                        m_Settings.m_PlaneMode = static_cast<EPlaneRenderingMode>(i);
                    }
                }
                ImGui::EndCombo();
            }

            ImGui::Checkbox("Play Recording", &m_Settings.m_IsPlayingRecording);

            ImGui::SliderFloat("Playback Speed", &m_Settings.m_PlaybackSpeed, 0.1f, 100.0f);

            m_Settings.m_SetRecordFile = false;

            char FileString[1024];
            std::strcpy(FileString, m_Settings.m_RecordFile.c_str());
            ImGui::InputText("Record File", FileString, 1024, ImGuiInputTextFlags_ReadOnly);

            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload * _pPayload = ImGui::AcceptDragDropPayload("ASSETS_DRAGDROP", 0))
                {
                    auto& DraggedAsset = *static_cast<Edit::CAsset*>(_pPayload->Data);

                    if (std::regex_match(DraggedAsset.GetPathToFile(), std::regex(".*.(swr)")))
                    {   
                        m_Settings.m_RecordFile = DraggedAsset.GetPathToFile();
                        m_Settings.m_SetRecordFile = true;
                    }
                    else
                    {
                        ENGINE_CONSOLE_ERROR("File is not a saltwater recording!");
                    }
                }

                ImGui::EndDragDropTarget();
            }

            m_Settings.m_Reset = (ImGui::Button("Reset Reconstruction"));

            ImGui::Checkbox("Render Volume", &m_Settings.m_RenderVolume);
            ImGui::Checkbox("Render Root", &m_Settings.m_RenderRoot);
            ImGui::Checkbox("Render Level 1", &m_Settings.m_RenderLevel1);
            ImGui::Checkbox("Render Level 2", &m_Settings.m_RenderLevel2);

            using Edit::CImFileFialog;
            using Edit::CAsset;

            if (ImGui::Button("Save recording"))
            {
                CImFileFialog::GetInstance().Open("Save record...", CAsset::s_Filter[CAsset::Record], Core::AssetManager::GetPathToAssets(), CImFileFialog::RootIsRoot | CImFileFialog::SaveDialog);
            }

            if (Edit::CImFileFialog::GetInstance().Draw())
            {
                auto Files = Edit::CImFileFialog::GetInstance().GetSelectedFiles();

                if (!Files.empty())
                {
                    auto rSceneFile = Files[0];

                    if (!regex_match(rSceneFile, CAsset::s_Filter[CAsset::Scene]))
                    {
                        rSceneFile += ".swr";
                    }

                    // TODO: implement
                }
            }

            return false;
        }

        // -----------------------------------------------------------------------------

        static void OnNewComponent(Dt::CEntity::BID _ID)
        {
            Dt::CEntity* pCurrentEntity = Dt::CEntityManager::GetInstance().GetEntityByID(_ID);

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
