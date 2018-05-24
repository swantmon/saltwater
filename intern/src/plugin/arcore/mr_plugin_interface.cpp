
#include "plugin/arcore/mr_precompiled.h"

#include "engine/engine.h"

#include "plugin/arcore/mr_camera.h"
#include "plugin/arcore/mr_control_manager.h"
#include "plugin/arcore/mr_plugin_interface.h"

CORE_PLUGIN_INFO(MR::CPluginInterface, "ArCore", "1.2.0", "This plugin use ARCore 1.2.0 to enable augmented reality features.")

namespace MR
{
    void CPluginInterface::OnStart()
    {
        // -----------------------------------------------------------------------------
        // Hooks
        // -----------------------------------------------------------------------------
        Engine::RegisterEventHandler(Engine::Gfx_OnUpdate, ENGINE_BIND_EVENT_METHOD(&CPluginInterface::Gfx_OnUpdate));

        // -----------------------------------------------------------------------------
        // Control manager
        // -----------------------------------------------------------------------------
        MR::ControlManager::OnStart();
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnExit()
    {
        MR::ControlManager::OnExit();
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::Update()
    {
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnPause()
    {
        MR::ControlManager::OnPause();
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnResume()
    {
        MR::ControlManager::OnResume();
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::Gfx_OnUpdate()
    {
        MR::ControlManager::Update();
    }
} // namespace MR