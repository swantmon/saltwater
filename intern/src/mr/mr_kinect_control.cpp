
#include "mr/mr_precompiled.h"

#include "mr/mr_kinect_control.h"

#include <iostream>
#include <type_traits>

#pragma comment(lib,"Kinect20.lib")
#pragma comment(lib,"Kinect20.fusion.lib")

namespace
{
    void CheckResult(HRESULT Result, char* pMessage)
    {
        if (Result != S_OK)
        {
            BASE_CONSOLE_ERROR(pMessage);
            throw std::exception(pMessage);
        }
    }

    template<typename T>
    void SafeRelease(T*& rInterface)
    {
        if (rInterface != nullptr)
        {
            rInterface->Release();
            rInterface = nullptr;
        }
    }
}

namespace MR
{
    CKinectControl::CKinectControl()
        : m_pKinect               (nullptr)
        , m_pDepthFrameReader     (nullptr)
    {
        Start();
    }

    // -----------------------------------------------------------------------------

    CKinectControl::~CKinectControl()
    {
        Stop();
    }

    // -----------------------------------------------------------------------------

    void CKinectControl::Start()
    {
        ////////////////////////////////////////////////////////////////////////////////////////////
        // Initialize kinect
        ////////////////////////////////////////////////////////////////////////////////////////////

        CheckResult(GetDefaultKinectSensor(&m_pKinect), "Failed to get default kinect");

        IColorFrameSource* pColorFrameSource = nullptr;
        IDepthFrameSource* pDepthFrameSource = nullptr;

        CheckResult(m_pKinect->Open(), "failed to open kinect");

        CheckResult(m_pKinect->get_ColorFrameSource(&pColorFrameSource), "Failed to get color frame source");
        CheckResult(pColorFrameSource->OpenReader(&m_pColorFrameReader), "Failed to open color frame reader");

        CheckResult(m_pKinect->get_DepthFrameSource(&pDepthFrameSource), "Failed to get depth frame source");
        CheckResult(pDepthFrameSource->OpenReader(&m_pDepthFrameReader), "Failed to open depth frame reader");
        
        SafeRelease(pColorFrameSource);
        SafeRelease(pDepthFrameSource);
    }

    // -----------------------------------------------------------------------------

    void CKinectControl::Stop()
    {
        SafeRelease(m_pDepthFrameReader);
        if (m_pKinect != nullptr)
        {
            m_pKinect->Close();
        }
        SafeRelease(m_pKinect);
    }

    // -----------------------------------------------------------------------------

    int CKinectControl::GetCameraWidth() const
    {
        return 0;
    }

    // -----------------------------------------------------------------------------

    int CKinectControl::GetCameraHeight() const
    {
        return 0;
    }

    // -----------------------------------------------------------------------------

    int CKinectControl::GetCameraPixelCount() const
    {
        return GetCameraWidth() * GetCameraHeight();
    }

    // -----------------------------------------------------------------------------

    float CKinectControl::GetCameraFocalLengthX() const
    {
        return 0.0f;
    }

    // -----------------------------------------------------------------------------

    float CKinectControl::GetCameraFocalLengthY() const
    {
        return 0.0f;
    }

    // -----------------------------------------------------------------------------

    float CKinectControl::GetCameraFocalPointX() const
    {
        return 0.0f;
    }

    // -----------------------------------------------------------------------------

    float CKinectControl::GetCameraFocalPointY() const
    {
        return 0.0f;
    }

    // -----------------------------------------------------------------------------
    
    bool CKinectControl::GetCameraFrame(unsigned char* pBuffer)
    {
        IColorFrame* pColorFrame = nullptr;
        unsigned int BufferSize;
        BYTE* pByteBuffer;

        if (m_pColorFrameReader->AcquireLatestFrame(&pColorFrame) != S_OK)
        {
            return false;
        }
        
        if (pColorFrame->AccessRawUnderlyingBuffer(&BufferSize, &pByteBuffer) != S_OK)
        {
            BASE_CONSOLE_ERROR("Failed to access underlying buffer");
            return false;
        }

        for (int i = 0; i < GetDepthPixelCount(); ++i)
        {
            pBuffer[i] = pByteBuffer[i];
        }

        pColorFrame->Release();

        return true;
    }

    // -----------------------------------------------------------------------------

    bool CKinectControl::GetDepthBuffer(unsigned short* pBuffer)
    {
        IDepthFrame* pDepthFrame = nullptr;
        unsigned int BufferSize;
        unsigned short* pShortBuffer;

        if (m_pDepthFrameReader->AcquireLatestFrame(&pDepthFrame) != S_OK)
        {
            return false;
        }

        if (pDepthFrame->AccessUnderlyingBuffer(&BufferSize, &pShortBuffer) != S_OK)
        {
            BASE_CONSOLE_ERROR("Failed to access underlying buffer");
            return false;
        }

        for (int i = 0; i < GetDepthPixelCount(); ++i)
        {
            pBuffer[i] = pShortBuffer[i];
        }

        pDepthFrame->Release();

        return true;
    }

    // -----------------------------------------------------------------------------

    int CKinectControl::GetDepthWidth() const
    {
        return NUI_DEPTH_RAW_WIDTH;
    }

    // -----------------------------------------------------------------------------

    int CKinectControl::GetDepthHeight() const
    {
        return NUI_DEPTH_RAW_HEIGHT;
    }

    // -----------------------------------------------------------------------------

    int CKinectControl::GetDepthPixelCount() const
    {
        return GetDepthWidth() * GetDepthHeight();
    }

    // -----------------------------------------------------------------------------

    float CKinectControl::GetDepthFocalLengthX() const
    {
        return NUI_KINECT_DEPTH_NORM_FOCAL_LENGTH_X * GetDepthWidth();
    }

    // -----------------------------------------------------------------------------

    float CKinectControl::GetDepthFocalLengthY() const
    {
        return NUI_KINECT_DEPTH_NORM_FOCAL_LENGTH_Y * GetDepthHeight();
    }

    // -----------------------------------------------------------------------------

    float CKinectControl::GetDepthFocalPointX() const
    {
        return NUI_KINECT_DEPTH_NORM_PRINCIPAL_POINT_X * GetDepthWidth();
    }

    // -----------------------------------------------------------------------------

    float CKinectControl::GetDepthFocalPointY() const
    {
        return NUI_KINECT_DEPTH_NORM_PRINCIPAL_POINT_Y * GetDepthHeight();
    }


} // namespace MR