
#include "mr/mr_precompiled.h"

#include "mr/mr_control_manager.h"
#include "mr/mr_realsense_control.h"

//#pragma comment(lib,"Kinect20.lib")
//#pragma comment(lib,"Kinect20.fusion.lib")

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