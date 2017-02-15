
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
        
        assert(m_pSenseManager != nullptr);

        m_pSenseManager->EnableStream(Capture::STREAM_TYPE_DEPTH, s_Width, s_Height, 30.0f);
        
        NSStatus::Status Result = m_pSenseManager->Init();

        m_pCaptureManager = m_pSenseManager->QueryCaptureManager();

        assert(m_pCaptureManager != nullptr);

        m_pDevice = m_pCaptureManager->QueryDevice();

        if (m_pDevice == nullptr)
        {
            throw std::exception("No Intel RealSense Found");
        }

        m_pDevice->SetMirrorMode(Device::MirrorMode::MIRROR_MODE_HORIZONTAL);
        
        m_FocalLength = m_pDevice->QueryDepthFocalLength();
        m_FocalPoint = m_pDevice->QueryDepthPrincipalPoint();  
    }

    // -----------------------------------------------------------------------------

    void CRealSenseControl::Stop()
    {
        SafeRelease(m_pDevice);
        SafeRelease(m_pCaptureManager);
        if (m_pSenseManager != nullptr)
        {
            m_pSenseManager->Close();
        }
        SafeRelease(m_pSenseManager);
    }

    // -----------------------------------------------------------------------------

    bool CRealSenseControl::GetDepthBuffer(unsigned short* pBuffer)
    {
        if (m_pSenseManager->AcquireFrame(true, 1000) != NSStatus::STATUS_NO_ERROR)
        {
            return false;
        }

        Capture::Sample* pSample = m_pSenseManager->QuerySample();

        Image* pDepth = pSample->depth;

        Image::ImageData data = {};
        data.format = Image::PIXEL_FORMAT_DEPTH_RAW;
        
        if (pDepth->AcquireAccess(Image::ACCESS_READ, &data) != NSStatus::STATUS_NO_ERROR)
        {
            return false;
        }
        
        unsigned short* pData = reinterpret_cast<unsigned short*>(data.planes[0]);

        for (int i = 0; i < GetWidth(); ++ i)
        {
            for (int j = 0; j < GetHeight(); ++ j)
            {
                pBuffer[j * GetWidth() + i] = pData[j * data.pitches[0] / 2 + i];
            }
        }

        pDepth->ReleaseAccess(&data);

        m_pSenseManager->ReleaseFrame();

        return true;
    }

    // -----------------------------------------------------------------------------

    int CRealSenseControl::GetWidth() const
    {
        return s_Width;
    }

    int CRealSenseControl::GetHeight() const
    {
        return s_Height;
    }

    // -----------------------------------------------------------------------------

    int CRealSenseControl::GetPixelCount() const
    {
        return GetWidth() * GetHeight();
    }

    // -----------------------------------------------------------------------------

    float CRealSenseControl::GetFocalLengthX() const
    {
        return m_FocalLength.x;
    }

    // -----------------------------------------------------------------------------

    float CRealSenseControl::GetFocalLengthY() const
    {
        return m_FocalLength.y;
    }

    // -----------------------------------------------------------------------------

    float CRealSenseControl::GetFocalPointX() const
    {
        return m_FocalPoint.x;
    }

    // -----------------------------------------------------------------------------

    float CRealSenseControl::GetFocalPointY() const
    {
        return m_FocalPoint.y;
    }
} // namespace MR