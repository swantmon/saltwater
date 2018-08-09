
#include "plugin/kinect/kinect_precompiled.h"

#include "engine/core/core_console.h"

#include "plugin/kinect/kinect_plugin_interface.h"

CORE_PLUGIN_INFO(HW::CPluginInterface, "Kinect", "1.0", "This plugin gives access to the MS Kinect.")

namespace HW
{
    void CPluginInterface::OnStart()
    {
        m_pControl.reset(new MR::CKinectControl);
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
        ENGINE_CONSOLE_INFOV("Kinect plugin paused!");
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnResume()
    {
        ENGINE_CONSOLE_INFOV("Kinect plugin resumed!");
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::GetDepthBuffer(unsigned short* pBuffer)
    {
        m_pControl->GetDepthBuffer(pBuffer);
    }
} // namespace HW

extern "C" CORE_PLUGIN_API_EXPORT void GetDepthBuffer(unsigned short* pBuffer)
{
    static_cast<HW::CPluginInterface&>(GetInstance()).GetDepthBuffer(pBuffer);
}