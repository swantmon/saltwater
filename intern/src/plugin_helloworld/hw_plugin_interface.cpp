
#include "plugin_helloworld/hw_precompiled.h"

#include "core/core_console.h"

#include "plugin_helloworld/hw_plugin_interface.h"

CORE_PLUGIN_INFO(HW::CPluginInterface, "Hello World", "1.0", "This is an example plugin w/o any useful features.")

namespace HW
{
    void CPluginInterface::OnStart()
    {
        ENGINE_CONSOLE_INFOV("Hello world plugin started!");
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnExit()
    {
        ENGINE_CONSOLE_INFOV("Hello world plugin exited!");
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::Update()
    {
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnPause()
    {
        ENGINE_CONSOLE_INFOV("Hello world plugin paused!");
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnResume()
    {
        ENGINE_CONSOLE_INFOV("Hello world plugin resumed!");
    }
} // namespace HW

extern "C" CORE_PLUGIN_API_EXPORT void SayHelloWorld()
{
    ENGINE_CONSOLE_INFOV("Hello world!");
}