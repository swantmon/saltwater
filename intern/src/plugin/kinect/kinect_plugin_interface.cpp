
#include "plugin/kinect/kinect_precompiled.h"

#include "engine/core/core_console.h"

#include "plugin/kinect/kinect_plugin_interface.h"

CORE_PLUGIN_INFO(HW::CPluginInterface, "Kinect", "1.0", "This plugin gives access to the MS Kinect.")

namespace HW
{
    void CPluginInterface::OnStart()
    {
        ENGINE_CONSOLE_INFOV("Kinect plugin started!");
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnExit()
    {
        ENGINE_CONSOLE_INFOV("Kinect plugin exited!");
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::Update()
    {
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnPause()
    {
        ENGINE_CONSOLE_INFOV("Kinect plugin paused!");
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnResume()
    {
        ENGINE_CONSOLE_INFOV("Kinect plugin resumed!");
    }
} // namespace HW

extern "C" CORE_PLUGIN_API_EXPORT void SayHelloWorld()
{
    ENGINE_CONSOLE_INFOV("Kinect!");
}