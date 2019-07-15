
#include "plugin/slam/slam_precompiled.h"

#include "engine/core/core_console.h"
#include "engine/engine.h"

#include "plugin/slam/slam_plugin_interface.h"

#include "plugin/slam/gfx_reconstruction_renderer.h"

#include <iostream>

CORE_PLUGIN_INFO(SLAM::CPluginInterface, "SLAM", "1.0", "This plugin provides Simultaneous Localization and Mapping based on a TSDF.")

namespace SLAM
{
    // -----------------------------------------------------------------------------

    void CPluginInterface::OnStart()
    {
        Gfx::ReconstructionRenderer::OnStart();

        m_UpdateDelegate        = Engine::RegisterEventHandler(Engine::EEvent::Gfx_OnUpdate, Gfx::ReconstructionRenderer::Update);
        m_RenderGBufferDelegate = Engine::RegisterEventHandler(Engine::EEvent::Gfx_OnRenderGBuffer, Gfx::ReconstructionRenderer::Render);
        m_RenderForwardDelegate = Engine::RegisterEventHandler(Engine::EEvent::Gfx_OnRenderForward, Gfx::ReconstructionRenderer::RenderForward);

        m_SLAMControl.Start();
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnExit()
    {
        m_SLAMControl.Exit();

        Gfx::ReconstructionRenderer::OnExit();
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::Update()
    {
        m_SLAMControl.Update();
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnInput(const Base::CInputEvent& _rEvent)
    {
        m_SLAMControl.OnInput(_rEvent);
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::UpdateScriptSettings(const Scpt::CSLAMScript::SScriptSettings& _rSettings)
    {
        if (_rSettings.m_Colorize || _rSettings.m_IsPermanentColorizationEnabled)
        {
            m_SLAMControl.ColorizePlanes();
        }

        m_SLAMControl.EnableMouseControl(_rSettings.m_IsMouseControlEnabled);
        m_SLAMControl.SetActivateSelection(_rSettings.m_IsSelectionEnabled);
        m_SLAMControl.SetIsPlaying(_rSettings.m_IsPlayingRecording);
        m_SLAMControl.SetPlaybackSpeed(_rSettings.m_PlaybackSpeed);

        if (_rSettings.m_SetRecordFile)
        {
            m_SLAMControl.SetRecordFile(Core::AssetManager::GetPathToAssets() + "/" + _rSettings.m_RecordFile, _rSettings.m_PlaybackSpeed);
            ENGINE_CONSOLE_INFOV("Playing recording from file \"%s\"", (Core::AssetManager::GetPathToAssets() + "/" + _rSettings.m_RecordFile).c_str());
        }

        Gfx::ReconstructionRenderer::SetVisibleObjects(
            _rSettings.m_RenderVolume,
            _rSettings.m_RenderRoot,
            _rSettings.m_RenderLevel1,
            _rSettings.m_RenderLevel2,
            _rSettings.m_PlaneMode
        );

		if (_rSettings.m_SendPlanes)
		{
			m_SLAMControl.SendPlanes();
		}

        if (_rSettings.m_Reset)
        {
            m_SLAMControl.ResetReconstruction();
        }
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::ReadScene(CSceneReader& _rCodec)
    {
        m_SLAMControl.ReadScene(_rCodec);
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::WriteScene(CSceneWriter& _rCodec)
    {
        m_SLAMControl.WriteScene(_rCodec);
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnPause()
    {
        ENGINE_CONSOLE_INFOV("SLAM plugin paused!");
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnResume()
    {
        ENGINE_CONSOLE_INFOV("SLAM plugin resumed!");
    }
} // namespace SLAM

extern "C" CORE_PLUGIN_API_EXPORT void Pause()
{
    static_cast<SLAM::CPluginInterface&>(GetInstance()).OnPause();
}

// -----------------------------------------------------------------------------

extern "C" CORE_PLUGIN_API_EXPORT void Resume()
{
    static_cast<SLAM::CPluginInterface&>(GetInstance()).OnResume();
}

// -----------------------------------------------------------------------------

extern "C" CORE_PLUGIN_API_EXPORT void Update()
{
    static_cast<SLAM::CPluginInterface&>(GetInstance()).Update();
}

// -----------------------------------------------------------------------------

extern "C" CORE_PLUGIN_API_EXPORT void OnInput(const Base::CInputEvent& _rEvent)
{
    static_cast<SLAM::CPluginInterface&>(GetInstance()).OnInput(_rEvent);
}

// -----------------------------------------------------------------------------

extern "C" CORE_PLUGIN_API_EXPORT void OnRenderHitProxy()
{
    Gfx::ReconstructionRenderer::RenderHitProxy();
}

// -----------------------------------------------------------------------------

extern "C" CORE_PLUGIN_API_EXPORT void UpdateScriptSettings(const Scpt::CSLAMScript::SScriptSettings& _rSettings)
{
    static_cast<SLAM::CPluginInterface&>(GetInstance()).UpdateScriptSettings(_rSettings);
}

// -----------------------------------------------------------------------------

extern "C" CORE_PLUGIN_API_EXPORT void ReadScene(CSceneReader& _rCodec)
{
    static_cast<SLAM::CPluginInterface&>(GetInstance()).ReadScene(_rCodec);
}

// -----------------------------------------------------------------------------

extern "C" CORE_PLUGIN_API_EXPORT void WriteScene(CSceneWriter& _rCodec)
{
    static_cast<SLAM::CPluginInterface&>(GetInstance()).WriteScene(_rCodec);
}