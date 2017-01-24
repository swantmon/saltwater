
#include "mr/mr_precompiled.h"

#include "base/base_console.h"
#include "base/base_memory.h"

#include "data/data_texture_manager.h"

#include "mr/cv_guidedfilter.h"
#include "mr/mr_control_manager.h"
#include "mr/mr_kinect_control.h"

#include "opencv2/opencv.hpp"
#include "Kinect.h"
#include <AR/video.h>

#define COLOR_WIDTH  1920
#define COLOR_HEIGHT 1080
#define DEPTH_WIDTH  512
#define DEPTH_HEIGHT 424

IKinectSensor*     g_pSensor;
IColorFrameReader* g_pColorFrameReader;
IDepthFrameReader* g_pDepthFrameReader;
ICoordinateMapper* g_pCoordinateMapper;
unsigned char      g_pColorFrameDataRAW[COLOR_WIDTH * COLOR_HEIGHT * 4];
float              g_pDepthFrameDataRAW[COLOR_WIDTH * COLOR_HEIGHT * 3];

cv::Mat DepthFC3     = cv::Mat(1080, 1920, CV_32FC1, &g_pDepthFrameDataRAW);
cv::Mat ColorUC4     = cv::Mat(1080, 1920, CV_8UC4, &g_pColorFrameDataRAW);
cv::Mat ColorUC3     = cv::Mat(1080, 1920, CV_8UC3, 0);
cv::Mat ColorUC1Crop = cv::Mat(720, 1280, CV_8UC1, 0);
cv::Mat DepthFC3Crop = cv::Mat(720, 1280, CV_32FC1, 0);
cv::Mat ColorUC3Crop = cv::Mat(720, 1280, CV_8UC3, 0);

namespace
{
    std::string g_PathToAssets = "../assets/";
} // namespace

namespace MR
{
    CKinectControl::CKinectControl()
        : CControl(CControl::Kinect)
    {
    }

    // -----------------------------------------------------------------------------

    CKinectControl::~CKinectControl()
    {
    }

    // -----------------------------------------------------------------------------

    void* CKinectControl::GetOriginalDepthFrame() const
    {
        return static_cast<void*>(static_cast<Base::Ptr>(DepthFC3.data));
    }

    // -----------------------------------------------------------------------------

    void* CKinectControl::GetConvertedDepthFrame() const
    {
        return static_cast<void*>(static_cast<Base::Ptr>(DepthFC3Crop.data));
    }

    // -----------------------------------------------------------------------------

    void CKinectControl::InternStart(const Base::Char* _pCameraParameterFile)
    {
        int Error;

        std::string PathToResource;

        // -----------------------------------------------------------------------------
        // Save configuration
        // -----------------------------------------------------------------------------
        Base::Int2 OriginalSize  = Base::Int2(1920, 1080);

        // -----------------------------------------------------------------------------
        // Load the camera parameters from file and save to camera parameters
        // -----------------------------------------------------------------------------
        ARParam NativeParams;

        PathToResource = g_PathToAssets + _pCameraParameterFile;

        Error = arParamLoad(PathToResource.c_str(), 1, &NativeParams);

        assert(Error >= 0);

        arParamChangeSize(&NativeParams, OriginalSize[0], OriginalSize[1], &NativeParams);

        assert(OriginalSize[0] == NativeParams.xsize && OriginalSize[1] == NativeParams.ysize);

        m_CameraParameters.m_FrameWidth = NativeParams.xsize;
        m_CameraParameters.m_FrameHeight = NativeParams.ysize;

        m_CameraParameters.m_ProjectionMatrix[0][0] = NativeParams.mat[0][0];
        m_CameraParameters.m_ProjectionMatrix[0][1] = NativeParams.mat[0][1];
        m_CameraParameters.m_ProjectionMatrix[0][2] = NativeParams.mat[0][2];
        m_CameraParameters.m_ProjectionMatrix[0][3] = NativeParams.mat[0][3];

        m_CameraParameters.m_ProjectionMatrix[1][0] = NativeParams.mat[1][0];
        m_CameraParameters.m_ProjectionMatrix[1][1] = NativeParams.mat[1][1];
        m_CameraParameters.m_ProjectionMatrix[1][2] = NativeParams.mat[1][2];
        m_CameraParameters.m_ProjectionMatrix[1][3] = NativeParams.mat[1][3];

        m_CameraParameters.m_ProjectionMatrix[2][0] = NativeParams.mat[2][0];
        m_CameraParameters.m_ProjectionMatrix[2][1] = NativeParams.mat[2][1];
        m_CameraParameters.m_ProjectionMatrix[2][2] = NativeParams.mat[2][2];
        m_CameraParameters.m_ProjectionMatrix[2][3] = NativeParams.mat[2][3];

        m_CameraParameters.m_DistortionFactor[0] = NativeParams.dist_factor[0];
        m_CameraParameters.m_DistortionFactor[1] = NativeParams.dist_factor[1];
        m_CameraParameters.m_DistortionFactor[2] = NativeParams.dist_factor[2];
        m_CameraParameters.m_DistortionFactor[3] = NativeParams.dist_factor[3];
        m_CameraParameters.m_DistortionFactor[4] = NativeParams.dist_factor[4];
        m_CameraParameters.m_DistortionFactor[5] = NativeParams.dist_factor[5];
        m_CameraParameters.m_DistortionFactor[6] = NativeParams.dist_factor[6];
        m_CameraParameters.m_DistortionFactor[7] = NativeParams.dist_factor[7];
        m_CameraParameters.m_DistortionFactor[8] = NativeParams.dist_factor[8];

        m_CameraParameters.m_DistortionFunctionVersion = NativeParams.dist_function_version;

        m_CameraParameters.m_PixelFormat = 1;

        // -----------------------------------------------------------------------------
        // Set projection matrix
        // -----------------------------------------------------------------------------
        m_ProjectionMatrix.SetIdentity();

        m_ProjectionMatrix[0][0] = static_cast<float>(m_CameraParameters.m_ProjectionMatrix[0][0]);
        m_ProjectionMatrix[0][1] = static_cast<float>(m_CameraParameters.m_ProjectionMatrix[0][1]);
        m_ProjectionMatrix[0][2] = static_cast<float>(m_CameraParameters.m_ProjectionMatrix[0][2]);

        m_ProjectionMatrix[1][0] = static_cast<float>(m_CameraParameters.m_ProjectionMatrix[1][0]);
        m_ProjectionMatrix[1][1] = static_cast<float>(m_CameraParameters.m_ProjectionMatrix[1][1]);
        m_ProjectionMatrix[1][2] = static_cast<float>(m_CameraParameters.m_ProjectionMatrix[1][2]);

        m_ProjectionMatrix[2][0] = static_cast<float>(m_CameraParameters.m_ProjectionMatrix[2][0]);
        m_ProjectionMatrix[2][1] = static_cast<float>(m_CameraParameters.m_ProjectionMatrix[2][1]);
        m_ProjectionMatrix[2][2] = static_cast<float>(m_CameraParameters.m_ProjectionMatrix[2][2]);

        // -----------------------------------------------------------------------------
        // Start capturing
        // -----------------------------------------------------------------------------
        HRESULT Result;

        // -----------------------------------------------------------------------------
        // Get default sensor
        // -----------------------------------------------------------------------------
        Result = GetDefaultKinectSensor(&g_pSensor);

        if (FAILED(Result))
        {
            BASE_CONSOLE_INFO("Getting default kinect failed.");
        }

        // -----------------------------------------------------------------------------
        // Startup sensor
        // -----------------------------------------------------------------------------
        g_pSensor->Open();

        g_pSensor->get_CoordinateMapper(&g_pCoordinateMapper);

        // -----------------------------------------------------------------------------
        // Start streams
        // -----------------------------------------------------------------------------
        IColorFrameSource* pColorFrameSource = 0;

        g_pSensor->get_ColorFrameSource(&pColorFrameSource);

        pColorFrameSource->OpenReader(&g_pColorFrameReader);

        if (g_pColorFrameReader != 0)
        {
            pColorFrameSource->Release();

            pColorFrameSource = 0;
        }

        // -----------------------------------------------------------------------------

        IDepthFrameSource* pDepthFrameSource = 0;

        g_pSensor->get_DepthFrameSource(&pDepthFrameSource);

        if (pDepthFrameSource != 0)
        {
            pDepthFrameSource->OpenReader(&g_pDepthFrameReader);

            if (g_pDepthFrameReader != 0)
            {
                g_pDepthFrameReader->Release();

                g_pDepthFrameReader = 0;
            }
        }

        // -----------------------------------------------------------------------------
        // Setup intern data
        // -----------------------------------------------------------------------------
        Dt::STextureDescriptor TextureDescriptor;

        TextureDescriptor.m_NumberOfPixelsU  = OriginalSize[0];
        TextureDescriptor.m_NumberOfPixelsV  = OriginalSize[1];
        TextureDescriptor.m_NumberOfPixelsW  = 1;
        TextureDescriptor.m_Format           = Dt::CTextureBase::R8G8B8_UBYTE;
        TextureDescriptor.m_Semantic         = Dt::CTextureBase::Diffuse;
        TextureDescriptor.m_Binding          = Dt::CTextureBase::CPU;
        TextureDescriptor.m_pPixels          = 0;
        TextureDescriptor.m_pFileName        = 0;
        TextureDescriptor.m_pIdentifier      = "ID_Webcam_RGB_Original_Output";

        m_pOriginalFrame = Dt::TextureManager::CreateTexture2D(TextureDescriptor);

        Dt::TextureManager::MarkTextureAsDirty(m_pOriginalFrame, Dt::CTextureBase::DirtyCreate);
    }

    // -----------------------------------------------------------------------------

    void CKinectControl::InternStop()
    {
        // -----------------------------------------------------------------------------
        // Shutdown streams
        // -----------------------------------------------------------------------------
        if (g_pColorFrameReader != 0)
        {
            g_pColorFrameReader->Release();

            g_pColorFrameReader = 0;
        }

        // -----------------------------------------------------------------------------

        if (g_pDepthFrameReader != 0)
        {
            g_pDepthFrameReader->Release();

            g_pDepthFrameReader = 0;
        }

        // -----------------------------------------------------------------------------
        // Shutdown device
        // -----------------------------------------------------------------------------
        if (g_pSensor != 0)
        {
            g_pSensor->Close();

            g_pSensor->Release();

            g_pSensor = 0;
        }
    }

    // -----------------------------------------------------------------------------

    void CKinectControl::InternUpdate()
    {
        HRESULT Result;

        IColorFrame* pColorFrame = 0;

        Result = g_pColorFrameReader->AcquireLatestFrame(&pColorFrame);

        if (SUCCEEDED(Result))
        {
            assert(pColorFrame);

            pColorFrame->CopyConvertedFrameDataToArray(COLOR_WIDTH * COLOR_HEIGHT * 4, g_pColorFrameDataRAW, ColorImageFormat_Rgba);
        }

        if (pColorFrame)
        {
            pColorFrame->Release();

            pColorFrame = 0;
        }

        // -----------------------------------------------------------------------------

        IDepthFrame* pDepthFrame = 0;

        Result = g_pDepthFrameReader->AcquireLatestFrame(&pDepthFrame);

        if (SUCCEEDED(Result))
        {
            assert(pDepthFrame);

            unsigned int Capacity;
            unsigned short* pBuffer;

            pDepthFrame->AccessUnderlyingBuffer(&Capacity, &pBuffer);

            static CameraSpacePoint CameraSpaceOfColorFrame[COLOR_WIDTH * COLOR_HEIGHT];

            g_pCoordinateMapper->MapColorFrameToCameraSpace(
                DEPTH_WIDTH * DEPTH_HEIGHT, pBuffer,
                COLOR_WIDTH * COLOR_HEIGHT, CameraSpaceOfColorFrame);

            assert(Capacity == DEPTH_HEIGHT * DEPTH_WIDTH);

            Base::CMemory::Copy(g_pDepthFrameDataRAW, CameraSpaceOfColorFrame, COLOR_WIDTH * COLOR_HEIGHT * 3 * sizeof(float));
        }

        if (pDepthFrame)
        {
            pDepthFrame->Release();

            pDepthFrame = 0;
        }

        // -----------------------------------------------------------------------------
        // Prepare OpenCV data
        // -----------------------------------------------------------------------------
        DepthFC3 = cv::Mat(1080, 1920, CV_32FC3, &g_pDepthFrameDataRAW);
        ColorUC4 = cv::Mat(1080, 1920, CV_8UC4, &g_pColorFrameDataRAW);

        //cv::flip(DepthFC1, DepthFC1, 1);
        //cv::flip(ColorUC4, ColorUC4, 1);

        cv::cvtColor(ColorUC4, ColorUC3, CV_RGBA2RGB);

        ColorUC3(cv::Rect(320, 180, 1280, 720)).copyTo(ColorUC3Crop);
        DepthFC3(cv::Rect(320, 180, 1280, 720)).copyTo(DepthFC3Crop);

        cv::flip(DepthFC3Crop, DepthFC3Crop, 1);
        cv::flip(ColorUC3Crop, ColorUC3Crop, 1);
        cv::flip(ColorUC3    , ColorUC3    , 1);

        // -----------------------------------------------------------------------------
        // Copy final result to the textures
        // -----------------------------------------------------------------------------
        Dt::TextureManager::CopyToTexture2D(m_pOriginalFrame, static_cast<void*>(static_cast<Base::Ptr>(ColorUC3.data)));

        Dt::TextureManager::CopyToTexture2D(m_pConvertedFrame, static_cast<void*>(static_cast<Base::Ptr>(ColorUC3Crop.data)));


        Dt::TextureManager::MarkTextureAsDirty(m_pConvertedFrame, Dt::CTextureBase::DirtyData);
    }
} // namespace MR