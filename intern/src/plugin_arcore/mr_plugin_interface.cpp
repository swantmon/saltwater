
#include "plugin_arcore/mr_precompiled.h"

#include "plugin_arcore/mr_camera.h"
#include "plugin_arcore/mr_control_manager.h"
#include "plugin_arcore/mr_plugin_interface.h"

CORE_PLUGIN_INFO(MR::CPluginInterface, "ArCore", "1.0")

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

extern "C" CORE_PLUGIN_API_EXPORT const MR::CCamera* GetCamera()
{
    return &MR::ControlManager::GetCamera();
}

extern "C" CORE_PLUGIN_API_EXPORT MR::CCamera::ETrackingState GetCameraTrackingState(const MR::CCamera* _pCamera)
{
    return _pCamera->GetTackingState();
}