
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

    void CPluginInterface::SetActivateSelection(bool _Flag)
    {
        m_SLAMControl.SetActivateSelection(_Flag);
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::EnableMouseControl(bool _Flag)
    {
        m_SLAMControl.EnableMouseControl(_Flag);
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
} // namespace HW

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

extern "C" CORE_PLUGIN_API_EXPORT void SetActivateSelection(bool _Flag)
{
    static_cast<SLAM::CPluginInterface&>(GetInstance()).SetActivateSelection(_Flag);
}

// -----------------------------------------------------------------------------

extern "C" CORE_PLUGIN_API_EXPORT void EnableMouseControl(bool _Flag)
{
    static_cast<SLAM::CPluginInterface&>(GetInstance()).EnableMouseControl(_Flag);
}