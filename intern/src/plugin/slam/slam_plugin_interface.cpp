
#include "plugin/slam/slam_precompiled.h"

#include "engine/core/core_console.h"
#include "engine/engine.h"

#include "plugin/slam/slam_plugin_interface.h"

#include "plugin/slam/gfx_reconstruction_renderer.h"

CORE_PLUGIN_INFO(HW::CPluginInterface, "SLAM", "1.0", "This plugin use an awesome Simultaneous Localization and Mapping.")

namespace HW
{
    void CPluginInterface::OnStart()
    {
        Gfx::ReconstructionRenderer::OnStart();
        Gfx::ReconstructionRenderer::OnSetupShader();
        Gfx::ReconstructionRenderer::OnSetupKernels();
        Gfx::ReconstructionRenderer::OnSetupRenderTargets();
        Gfx::ReconstructionRenderer::OnSetupStates();
        Gfx::ReconstructionRenderer::OnSetupTextures();
        Gfx::ReconstructionRenderer::OnSetupBuffers();
        Gfx::ReconstructionRenderer::OnSetupResources();
        Gfx::ReconstructionRenderer::OnSetupModels();
        Gfx::ReconstructionRenderer::OnSetupEnd();

        Engine::RegisterEventHandler(Engine::Gfx_OnUpdate, Gfx::ReconstructionRenderer::Update);
        Engine::RegisterEventHandler(Engine::Gfx_OnRenderGBuffer, Gfx::ReconstructionRenderer::Render);
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnExit()
    {
        Gfx::ReconstructionRenderer::OnExit();
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::Update()
    {
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

extern "C" CORE_PLUGIN_API_EXPORT void SayHelloWorld()
{
    ENGINE_CONSOLE_INFOV("SLAM!");
}