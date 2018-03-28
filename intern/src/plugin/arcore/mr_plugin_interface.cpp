
#include "plugin/arcore/mr_precompiled.h"

#include "plugin/arcore/mr_control_manager.h"
#include "plugin/arcore/mr_plugin_interface.h"

Core::IPlugin& GetInstance()
{
    static MR::CPluginInterface s_Instance;
    return s_Instance;
}

struct SInternalPluginInfo
{
    Core::SPluginInfo InfoExport =
    {
        CORE_PLUGIN_API_VERSION,
        __FILE__,
        "MR::CPluginInterface",
        "ArCore",
        "1.0",
        GetInstance,
    };

    SInternalPluginInfo()
    {
        Core::CPluginManager::GetInstance().RegisterPlugin(InfoExport);
    }
} const g_ArCoreInternalPluginInfo;

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