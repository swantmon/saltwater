
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
    }

    // -----------------------------------------------------------------------------

    CKinectControl::~CKinectControl()
    {
    }

    // -----------------------------------------------------------------------------

    void CKinectControl::Start()
    {
        ////////////////////////////////////////////////////////////////////////////////////////////
        // Initialize kinect
        ////////////////////////////////////////////////////////////////////////////////////////////

        CheckResult(GetDefaultKinectSensor(&m_pKinect), "Failed to get default kinect");

        // Initialize the Kinect and get the depth reader
        IDepthFrameSource* pDepthFrameSource = nullptr;

        CheckResult(m_pKinect->Open(), "failed to open kinect");

        CheckResult(m_pKinect->get_DepthFrameSource(&pDepthFrameSource), "Failed to get depth frame source");

        CheckResult(pDepthFrameSource->OpenReader(&m_pDepthFrameReader), "Failed to open depth frame reader");

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

        for (int i = 0; i < GetPixelCount(); ++i)
        {
            pBuffer[i] = pShortBuffer[i];
        }


        pDepthFrame->Release();

        return true;
    }

    int CKinectControl::GetWidth()
    {
        return NUI_DEPTH_RAW_WIDTH;
    }

    int CKinectControl::GetHeight()
    {
        return NUI_DEPTH_RAW_HEIGHT;
    }

    int CKinectControl::GetPixelCount()
    {
        return GetWidth() * GetHeight();
    }

    float CKinectControl::GetFocalLengthX()
    {
        return NUI_KINECT_DEPTH_NORM_FOCAL_LENGTH_X * GetWidth();
    }

    float CKinectControl::GetFocalLengthY()
    {
        return NUI_KINECT_DEPTH_NORM_FOCAL_LENGTH_Y * GetHeight();
    }

    float CKinectControl::GetFocalPointX()
    {
        return NUI_KINECT_DEPTH_NORM_PRINCIPAL_POINT_X * GetWidth();
    }

    float CKinectControl::GetFocalPointY()
    {
        return NUI_KINECT_DEPTH_NORM_PRINCIPAL_POINT_Y * GetHeight();
    }


} // namespace MR