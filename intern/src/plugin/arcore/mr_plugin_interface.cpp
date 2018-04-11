
#include "plugin/arcore/mr_precompiled.h"

#include "plugin/arcore/mr_camera.h"
#include "plugin/arcore/mr_control_manager.h"
#include "plugin/arcore/mr_plugin_interface.h"

CORE_PLUGIN_INFO(MR::CPluginInterface, "ArCore", "1.0", "This plugin use ARCore to enable augmented reality features.")

namespace MR
{
    void CPluginInterface::OnStart()
    {
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
        MR::ControlManager::Update();
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
} // namespace MR