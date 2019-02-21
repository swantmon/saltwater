
#include "plugin/stereo_matching/stereo_precompiled.h"

#include "engine/core/core_console.h"

#include "engine/engine.h"

#include "plugin/stereo_matching/stereo_plugin_interface.h"

CORE_PLUGIN_INFO(HW::CPluginInterface, "Stereo Matching", "1.0", "This plugin takes RGB and transformation data and provides 2.5D depth maps")

namespace HW
{
    void CPluginInterface::OnStart()
    {
        ENGINE_CONSOLE_INFOV("Stereo matching plugin started!");
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnExit()
    {
        ENGINE_CONSOLE_INFOV("Stereo matching plugin exited!");
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::Update()
    {
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnPause()
    {
        ENGINE_CONSOLE_INFOV("Stereo matching plugin paused!");
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnResume()
    {
        ENGINE_CONSOLE_INFOV("Stereo matching plugin resumed!");
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::EventHook()
    {
        ENGINE_CONSOLE_INFOV("Oh hello. An event has been raised!");
    }
} // namespace HW

extern "C" CORE_PLUGIN_API_EXPORT void SayHelloWorld()
{
    ENGINE_CONSOLE_INFOV("Hello world!");
}