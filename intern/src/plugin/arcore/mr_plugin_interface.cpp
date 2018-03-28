
#include "plugin/arcore/mr_precompiled.h"

#include "plugin/arcore/mr_control_manager.h"
#include "plugin/arcore/mr_plugin_interface.h"

CORE_PLUGIN(MR::CPluginInterface, "ArCore", "1.0")

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
} // namespace MR