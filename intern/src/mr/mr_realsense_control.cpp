
#include "mr/mr_precompiled.h"

#include "mr/mr_control_manager.h"
#include "mr/mr_realsense_control.h"

#include "RealSense/SenseManager.h"

#ifdef _DEBUG
#pragma comment(lib,"libpxc_d.lib")
#pragma comment(lib,"libpxcmd_d.lib")
#else
#pragma comment(lib,"libpxc.lib")
#pragma comment(lib,"libpxcmd.lib")
#endif

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
        Intel::RealSense::SenseManager* pSenseManager = Intel::RealSense::SenseManager::CreateInstance();
    }

    // -----------------------------------------------------------------------------

    void CRealSenseControl::Stop()
    {

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