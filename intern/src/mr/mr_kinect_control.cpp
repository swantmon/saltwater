
#include "mr/mr_precompiled.h"

#include "mr/mr_kinect_control.h"

#include "base/base_program_parameters.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <type_traits>

#pragma comment(lib,"Kinect20.lib")
#pragma comment(lib,"Kinect20.fusion.lib")

namespace
{
    std::string g_FilePath = "..//data//slam//datasets//";

    std::vector<UINT16> TotalDepthBuffers;
    int LoadedFrameCount = 0;
    int CurrentLoadedFrame = 0;

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
        , m_pMapper               (nullptr)
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
        int StoreMode = Base::CProgramParameters::GetInstance().Get("mr:slam:storing:mode", 0);
        m_DatasetPath = Base::CProgramParameters::GetInstance().Get("mr:slam:storing:path", "dataset");
        g_StoreFrames = StoreMode == 1;
        g_LoadFrames = StoreMode == 2;

        assert(!(g_LoadFrames && g_StoreFrames));

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
                
        m_pKinect->get_CoordinateMapper(&m_pMapper);

        m_CameraFrameBuffer = std::vector<Byte4>(GetCameraWidth() * GetCameraHeight());
        m_DepthBuffer = std::vector<UINT16>(GetDepthWidth() * GetDepthHeight());
        m_ColorSpacePoints = std::vector<ColorSpacePoint>(GetDepthWidth() * GetDepthHeight());

        LoadedFrameCount = 0;
        CurrentLoadedFrame = 0;

        if (g_LoadFrames)
        {
            for (;;)
            {
                std::stringstream FileName;
                FileName << g_FilePath << m_DatasetPath << LoadedFrameCount << ".txt";

                std::ifstream File;
                File.open(FileName.str(), std::ios::binary);

                if (File.is_open())
                {
                    ++LoadedFrameCount;
                    TotalDepthBuffers.resize(LoadedFrameCount * GetDepthPixelCount());

                    int TotalIndex = (LoadedFrameCount - 1) * GetDepthPixelCount();
                    File.read(reinterpret_cast<char*>(&TotalDepthBuffers[TotalIndex]), sizeof(TotalDepthBuffers[0]) * GetDepthPixelCount());
                }
                else
                {
                    break;
                }
            }
        }
        if (g_StoreFrames)
        {
            TotalDepthBuffers.reserve(GetDepthPixelCount() * 2000);
        }
    }

    // -----------------------------------------------------------------------------

    void CKinectControl::Stop()
    {
        SafeRelease(m_pMapper);
        SafeRelease(m_pDepthFrameReader);
        if (m_pKinect != nullptr)
        {
            m_pKinect->Close();
        }
        SafeRelease(m_pKinect);
        
        if (g_StoreFrames)
        {
            for (int FrameIndex = 0; FrameIndex < TotalDepthBuffers.size() / GetDepthPixelCount(); ++FrameIndex)
            {
                std::stringstream FileName;
                FileName << g_FilePath << m_DatasetPath << FrameIndex << ".txt";

                std::ofstream File(FileName.str(), std::ios::binary);

                File.write(reinterpret_cast<char*>(&TotalDepthBuffers[FrameIndex * GetDepthPixelCount()]), sizeof(TotalDepthBuffers[0]) * GetDepthPixelCount());
            }
        }
    }

    // -----------------------------------------------------------------------------

    bool CKinectControl::GetDepthBuffer(unsigned short* pBuffer)
    {
        if (g_LoadFrames)
        {
            if (CurrentLoadedFrame < LoadedFrameCount)
            {
                for (int i = 0; i < GetDepthPixelCount(); ++i)
                {
                    pBuffer[i] = TotalDepthBuffers[CurrentLoadedFrame * GetDepthPixelCount() + i];
                }

                ++CurrentLoadedFrame;

                return true;
            }
            else
            {
                return false;
            }
        }
        else
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

            const int PixelCount = GetDepthPixelCount();
            
            std::memcpy(pBuffer, pShortBuffer, sizeof(pShortBuffer[0]) * PixelCount);
            std::memcpy(m_DepthBuffer.data(), pShortBuffer, sizeof(pShortBuffer[0]) * PixelCount);

            pDepthFrame->Release();
            
            if (g_StoreFrames)
            {
                for (auto Value : m_DepthBuffer)
                {
                    TotalDepthBuffers.push_back(Value);
                }
            }

            return true;
        }
    }

    // -----------------------------------------------------------------------------

    int CKinectControl::GetCameraWidth() const
    {
        return 1920;
    }

    // -----------------------------------------------------------------------------

    int CKinectControl::GetCameraHeight() const
    {
        return 1080;
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
    
    bool CKinectControl::GetCameraFrame(char* _pBuffer)
    {
        Byte4* pBuffer = reinterpret_cast<Byte4*>(_pBuffer);

        IColorFrame* pColorFrame = nullptr;

        if (m_pColorFrameReader->AcquireLatestFrame(&pColorFrame) != S_OK)
        {
            return false;
        }
        
        pColorFrame->CopyConvertedFrameDataToArray(
            GetCameraWidth() * GetCameraHeight() * sizeof(m_CameraFrameBuffer[0]),
            &m_CameraFrameBuffer[0].r,
            ColorImageFormat::ColorImageFormat_Rgba
        );

        pColorFrame->Release();
        
        HRESULT Result = m_pMapper->MapDepthFrameToColorSpace(
            static_cast<UINT>(m_DepthBuffer.size()), m_DepthBuffer.data(),
            static_cast<UINT>(m_ColorSpacePoints.size()), m_ColorSpacePoints.data()
        );
        
        if (Result != S_OK)
        {
            return false;
        }

        const int DepthWidth = GetDepthWidth();
        const int DepthHeight = GetDepthHeight();
        const int CameraWidth = GetCameraWidth();
        const int CameraHeight = GetCameraHeight();

        for (int i = 0; i < DepthWidth; ++ i)
        {
            for (int j = 0; j < DepthHeight; ++ j)
            {
                const int x = static_cast<int>(m_ColorSpacePoints[j * DepthWidth + i].X + 0.5f);
                const int y = static_cast<int>(m_ColorSpacePoints[j * DepthWidth + i].Y + 0.5f);

                if (x >= 0 && x < CameraWidth && y >= 0 && y < CameraHeight)
                {
                    pBuffer[j * DepthWidth + i] = m_CameraFrameBuffer[y * CameraWidth + x];
                }
                else
                {
                    pBuffer[j * DepthWidth + i] = { 0, 0, 0, 255 };
                }
            }
        }

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

	// -----------------------------------------------------------------------------

	float CKinectControl::GetMinDepth() const
	{
		return 0.5f;
	}

	// -----------------------------------------------------------------------------

	float CKinectControl::GetMaxDepth() const
	{
		return 8.0f;
	}

} // namespace MR