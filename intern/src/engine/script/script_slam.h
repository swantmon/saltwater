
#pragma once

#include "base/base_compression.h"
#include "base/base_exception.h"
#include "base/base_include_glm.h"

#include "engine/camera/cam_control_manager.h"
#include "engine/camera/cam_editor_control.h"

#include "engine/core/core_program_parameters.h"

#include "engine/core/core_plugin_manager.h"

#include "engine/engine.h"

#include "engine/graphic/gfx_context_manager.h"
#include "engine/graphic/gfx_shader_manager.h"
#include "engine/graphic/gfx_texture.h"
#include "engine/graphic/gfx_texture_manager.h"
#include "engine/graphic/gfx_view_manager.h"

#include "engine/script/script_script.h"

#include "engine/network/core_network_manager.h"

namespace Scpt
{
    class CSLAMScript : public CScript<CSLAMScript>
    {
    public:

        enum EPlaneRenderingMode
        {
            NONE,
            EXTENT_ONLY,
            MESH_ONLY,
            MESH_WITH_EXTENT,
            ALL
        };

        struct SScriptSettings
        {
            bool m_RenderVolume;

            bool m_RenderRoot;
            bool m_RenderLevel1;
            bool m_RenderLevel2;

            EPlaneRenderingMode m_PlaneMode;

            bool m_Reset;

            bool m_IsSelectionEnabled;
            bool m_IsMouseControlEnabled;
            bool m_IsPlayingRecording;
            bool m_IsPermanentColorizationEnabled;

            bool m_Colorize;

            bool m_SetRecordFile;
            std::string m_RecordFile;
            float m_PlaybackSpeed;
        };
        
        void Start() override
        {
            if (!Core::PluginManager::LoadPlugin("SLAM"))
            {
                throw Base::CException(__FILE__, __LINE__, "SLAM plugin could not be loaded");
            }
            
            InputCallback = (FInputCallback)(Core::PluginManager::GetPluginFunction("SLAM", "OnInput"));

            SetSettings = (FSetSettings)(Core::PluginManager::GetPluginFunction("SLAM", "UpdateScriptSettings"));

            m_Settings.m_RenderVolume = true;
            m_Settings.m_RenderRoot = false;
            m_Settings.m_RenderLevel1 = false;
            m_Settings.m_RenderLevel2 = false;
            m_Settings.m_PlaneMode = EPlaneRenderingMode::MESH_ONLY;
            m_Settings.m_IsSelectionEnabled = false;
            m_Settings.m_IsMouseControlEnabled = false;
            m_Settings.m_IsPlayingRecording = false;
            m_Settings.m_IsPermanentColorizationEnabled = false;
            m_Settings.m_Colorize = false;
            m_Settings.m_PlaybackSpeed = 1.0f;
        }

        // -----------------------------------------------------------------------------

        void Exit() override
        {
            
        }

        // -----------------------------------------------------------------------------

        void Update() override
        {
            SetSettings(m_Settings);
        }

        // -----------------------------------------------------------------------------

        void OnInput(const Base::CInputEvent& _rEvent) override
        {
            InputCallback(_rEvent);
        }
        
    private:

        using FInputCallback = void(*)(const Base::CInputEvent& _rEvent);
        FInputCallback InputCallback;

        using FSetSettings = void(*)(const SScriptSettings& _rSettings);
        FSetSettings SetSettings;

    protected:

        SScriptSettings m_Settings;
    };
} // namespace Scpt