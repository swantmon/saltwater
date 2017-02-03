
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

        m_pSenseManager->EnableStream(Capture::STREAM_TYPE_DEPTH, m_Width, m_Height, 30.0f);
        
        NSStatus::Status Result = m_pSenseManager->Init();

        m_pCaptureManager = m_pSenseManager->QueryCaptureManager();

        assert(m_pCaptureManager != nullptr);

        m_pDevice = m_pCaptureManager->QueryDevice();

        if (m_pDevice == nullptr)
        {
            throw std::exception("No Intel RealSense Found");
        }
        
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
        if (!m_pSenseManager->IsConnected())
        {
            throw std::exception("Lost connection to Intel RealSense");
        }

        return false;
    }

    // -----------------------------------------------------------------------------

    int CRealSenseControl::GetWidth()
    {
        return m_Width;
    }

    int CRealSenseControl::GetHeight()
    {
        return m_Height;
    }

    // -----------------------------------------------------------------------------

    int CRealSenseControl::GetPixelCount()
    {
        return GetWidth() * GetHeight();
    }

    // -----------------------------------------------------------------------------

    float CRealSenseControl::GetFocalLengthX()
    {
        return m_FocalLength.x;
    }

    // -----------------------------------------------------------------------------

    float CRealSenseControl::GetFocalLengthY()
    {
        return m_FocalLength.y;
    }

    // -----------------------------------------------------------------------------

    float CRealSenseControl::GetFocalPointX()
    {
        return m_FocalPoint.x;
    }

    // -----------------------------------------------------------------------------

    float CRealSenseControl::GetFocalPointY()
    {
        return m_FocalPoint.y;
    }
} // namespace MR