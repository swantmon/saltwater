
#include "mr/mr_precompiled.h"

#include "base/base_memory.h"

#include "core/core_time.h"

#include "data/data_texture_manager.h"

#include "mr/mr_control_manager.h"
#include "mr/mr_realsense_control.h"

#include <iostream>
#include <type_traits>

//#pragma comment(lib,"Kinect20.lib")
//#pragma comment(lib,"Kinect20.fusion.lib")

namespace MR
{
    const int CRealSenseControl::DepthImageWidth = 640;
    const int CRealSenseControl::DepthImageHeight = 480;
    const int CRealSenseControl::DepthImagePixelsCount = DepthImageWidth * DepthImageHeight;

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
} // namespace MR