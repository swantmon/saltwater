
#include "mr/mr_precompiled.h"

#include "mr/mr_control_manager.h"
#include "mr/mr_realsense_control.h"

using namespace Intel::RealSense;

namespace MR
{
    CRealSenseControl::CRealSenseControl()
    {
    }

    // -----------------------------------------------------------------------------

    CRealSenseControl::~CRealSenseControl()
    {
    }

    // -----------------------------------------------------------------------------

    void CRealSenseControl::Start()
    {
        m_pSenseManager = SenseManager::CreateInstance();
        Capture::DeviceInfo DeviceInfo = {};
        DeviceInfo.model = Capture::DEVICE_MODEL_R200_ENHANCED;
        m_pSenseManager->QueryCaptureManager()->FilterByDeviceInfo(&DeviceInfo);

        m_pSenseManager->EnableStream(Capture::STREAM_TYPE_DEPTH, 0, 0, 0);
        
        m_pSenseManager->Init();
    }

    // -----------------------------------------------------------------------------

    void CRealSenseControl::Stop()
    {
        m_pSenseManager->Release();
    }

    // -----------------------------------------------------------------------------

    bool CRealSenseControl::GetDepthBuffer(unsigned short* pBuffer)
    {
        return false;
    }

    // -----------------------------------------------------------------------------

    int CRealSenseControl::GetWidth()
    {
        return -1;
    }

    int CRealSenseControl::GetHeight()
    {
        return -1;
    }

    // -----------------------------------------------------------------------------

    int CRealSenseControl::GetPixelCount()
    {
        return -1;
    }

    // -----------------------------------------------------------------------------

    float CRealSenseControl::GetFocalLengthX()
    {
        return -1.0f;
    }

    // -----------------------------------------------------------------------------

    float CRealSenseControl::GetFocalLengthY()
    {
        return -1.0f;
    }

    // -----------------------------------------------------------------------------

    float CRealSenseControl::GetFocalPointX()
    {
        return -1.0f;
    }

    // -----------------------------------------------------------------------------

    float CRealSenseControl::GetFocalPointY()
    {
        return -1.0f;
    }
} // namespace MR