
#include "plugin/kinect/kinect_precompiled.h"

#include "engine/core/core_console.h"

#include "plugin/kinect/kinect_plugin_interface.h"

CORE_PLUGIN_INFO(HW::CPluginInterface, "Kinect", "1.0", "This plugin gives access to the MS Kinect v2.")

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

    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::OnResume()
    {

    }

    // -----------------------------------------------------------------------------

    bool CPluginInterface::GetDepthBuffer(unsigned short* pBuffer)
    {
        return m_pControl->GetDepthBuffer(pBuffer);
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::GetColorBuffer(char* pBuffer)
    {
        m_pControl->GetCameraFrame(pBuffer);
    }

    // -----------------------------------------------------------------------------

    void CPluginInterface::GetIntrinsics(glm::vec2& FocalLength, glm::vec2& FocalPoint, glm::ivec2& Size)
    {
        FocalLength = glm::vec2(m_pControl->GetDepthFocalLengthX(), m_pControl->GetDepthFocalLengthY());
        FocalPoint = glm::vec2(m_pControl->GetDepthFocalPointX(), m_pControl->GetDepthFocalPointY());
        Size = glm::ivec2(m_pControl->GetDepthWidth(), m_pControl->GetDepthHeight());
    }
} // namespace HW

extern "C" CORE_PLUGIN_API_EXPORT void GetColorBuffer(char* pBuffer)
{
    static_cast<HW::CPluginInterface&>(GetInstance()).GetColorBuffer(pBuffer);
}

extern "C" CORE_PLUGIN_API_EXPORT bool GetDepthBuffer(unsigned short* pBuffer)
{
    return static_cast<HW::CPluginInterface&>(GetInstance()).GetDepthBuffer(pBuffer);
}

extern "C" CORE_PLUGIN_API_EXPORT void GetIntrinsics(glm::vec2& FocalLength, glm::vec2& FocalPoint, glm::ivec2& Size)
{
    static_cast<HW::CPluginInterface&>(GetInstance()).GetIntrinsics(FocalLength, FocalPoint, Size);
}