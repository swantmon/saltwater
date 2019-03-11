
#include "plugin/pixmix/pm_precompiled.h"

#include "engine/core/core_console.h"

#include "engine/engine.h"

#include "plugin/pixmix/pm_plugin_interface.h"

CORE_PLUGIN_INFO(PM::CPluginInterface, "PixMix", "1.0", "This plugin enables inpainting with PixMix.")

namespace PM
{
    void CPluginInterface::OnStart()
    {
		
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnExit()
    {
		
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::Update()
    {
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnPause()
    {
		
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnResume()
    {
		
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::EventHook()
    {
		
    }
} // namespace HW

extern "C" CORE_PLUGIN_API_EXPORT void SayHelloWorld()
{
    ENGINE_CONSOLE_INFOV("Hello world!");
}