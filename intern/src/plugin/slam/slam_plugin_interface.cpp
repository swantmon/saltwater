
#include "plugin/slam/slam_precompiled.h"

#include "engine/core/core_console.h"

#include "plugin/slam/slam_plugin_interface.h"

CORE_PLUGIN_INFO(HW::CPluginInterface, "SLAM", "1.0", "This plugin use an awesome Simultaneous Localization and Mapping.")

namespace HW
{
    void CPluginInterface::OnStart()
    {
        ENGINE_CONSOLE_INFOV("SLAM plugin started!");
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnExit()
    {
        ENGINE_CONSOLE_INFOV("SLAM plugin exited!");
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