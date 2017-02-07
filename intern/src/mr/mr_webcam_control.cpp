
#include "mr/mr_precompiled.h"

#include "base/base_memory.h"

#include "data/data_texture_manager.h"

#include "mr/mr_control_manager.h"
#include "mr/mr_webcam_control.h"

#include <AR/video.h>

#include "opencv2/opencv.hpp"

namespace
{
	std::string g_PathToAssets = "../assets/";
} // namespace

namespace MR
{
    CWebcamControl::CWebcamControl()
        : CControl               (CControl::Webcam)
        , m_Configuration        ("-device=WinDS -flipV")
        , m_OriginalColorFrameBGR()
        , m_OriginalColorFrameRGB()
        , m_ConvertedColorFrame  ()
    {
    }

    // -----------------------------------------------------------------------------

    CWebcamControl::~CWebcamControl()
    {
    }

    // -----------------------------------------------------------------------------

    void CWebcamControl::SetConfiguration(const std::string& _rConfiguration)
    {
        m_Configuration = _rConfiguration;
    }

    // -----------------------------------------------------------------------------

    const std::string& CWebcamControl::GetConfiguration() const
    {
        return m_Configuration;
    }

    // -----------------------------------------------------------------------------

    void CWebcamControl::InternStart(const Base::Char* _pCameraParameterFile)
    {
        int Error;

        std::string PathToResource;

        // -----------------------------------------------------------------------------
        // Setup
        // -----------------------------------------------------------------------------
        Base::U16 ConvertedSizeU = m_pConvertedFrame->GetNumberOfPixelsU();
        Base::U16 ConvertedSizeV = m_pConvertedFrame->GetNumberOfPixelsV();

        int OriginalSizeU = 0;
        int OriginalSizeV = 0;

        // -----------------------------------------------------------------------------
        // Setup camera parameters and open video
        // -----------------------------------------------------------------------------
        Error = arVideoOpen(m_Configuration.c_str());

        assert(Error >= 0);

        // -----------------------------------------------------------------------------
        // Get original video size
        // -----------------------------------------------------------------------------
        Error = arVideoGetSize(&OriginalSizeU, &OriginalSizeV);

        assert(Error >= 0);

        // -----------------------------------------------------------------------------
        // Get original pixel format
        // -----------------------------------------------------------------------------
        int OriginalPixelFormat;

        OriginalPixelFormat = arVideoGetPixelFormat();

        assert(OriginalPixelFormat >= 0);

        // -----------------------------------------------------------------------------
        // Load the camera parameters from file and save to camera parameters
        // -----------------------------------------------------------------------------
        ARParam NativeParams;

		PathToResource = g_PathToAssets + _pCameraParameterFile;

        Error = arParamLoad(PathToResource.c_str(), 1, &NativeParams);

        assert(Error >= 0);

        arParamChangeSize(&NativeParams, OriginalSizeU, OriginalSizeV, &NativeParams);

        assert(OriginalSizeU == NativeParams.xsize && OriginalSizeV == NativeParams.ysize);

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

        m_CameraParameters.m_PixelFormat = OriginalPixelFormat;

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
        Error = arVideoCapStart();

        assert(Error == 0);

        // -----------------------------------------------------------------------------
        // Setup OpenCV images for further image processing
        // -----------------------------------------------------------------------------
        unsigned int NumberOfVideoComponents = 3;

#ifdef __APPLE__
        NumberOfVideoComponents = 4;
#endif

        m_OriginalColorFrameBGR = cvCreateImage(cvSize(OriginalSizeU, OriginalSizeV), IPL_DEPTH_8U, NumberOfVideoComponents);
        m_OriginalColorFrameRGB = cvCreateImage(cvSize(OriginalSizeU, OriginalSizeV), IPL_DEPTH_8U, 3);
        m_ConvertedColorFrame   = cvCreateImage(cvSize(ConvertedSizeU, ConvertedSizeV), IPL_DEPTH_8U, 3);

        // -----------------------------------------------------------------------------
        // Setup intern data
        // -----------------------------------------------------------------------------
        Dt::STextureDescriptor TextureDescriptor;

        TextureDescriptor.m_NumberOfPixelsU  = OriginalSizeU;
        TextureDescriptor.m_NumberOfPixelsV  = OriginalSizeV;
        TextureDescriptor.m_NumberOfPixelsW  = 1;
        TextureDescriptor.m_Format           = Dt::CTextureBase::R8G8B8_UBYTE;
        TextureDescriptor.m_Semantic         = Dt::CTextureBase::Diffuse;
        TextureDescriptor.m_Binding          = Dt::CTextureBase::CPU;
        TextureDescriptor.m_pPixels          = static_cast<IplImage*>(m_OriginalColorFrameRGB)->imageData;
        TextureDescriptor.m_pFileName        = 0;
        TextureDescriptor.m_pIdentifier      = "ID_Webcam_RGB_Original_Output";

        m_pOriginalFrame = Dt::TextureManager::CreateTexture2D(TextureDescriptor);

        Dt::TextureManager::MarkTextureAsDirty(m_pOriginalFrame, Dt::CTextureBase::DirtyCreate);

        // -----------------------------------------------------------------------------
        // Set data of original video / image
        // -----------------------------------------------------------------------------
        ARUint8* pVideoData = arVideoGetImage();

        for (; pVideoData == 0;)
        {
            pVideoData = arVideoGetImage();
        }

        static_cast<IplImage*>(m_OriginalColorFrameBGR)->imageData = static_cast<char*>(static_cast<void*>(pVideoData));

        // -----------------------------------------------------------------------------
        // Convert from original to output
        // -----------------------------------------------------------------------------
        ConvertOriginalToOutput();
    }

    // -----------------------------------------------------------------------------

    void CWebcamControl::InternStop()
    {
        arVideoCapStop();
        arVideoClose();
    }

    // -----------------------------------------------------------------------------

    void CWebcamControl::InternUpdate()
    {
        // -----------------------------------------------------------------------------
        // Get new video image from ARToolkit
        // -----------------------------------------------------------------------------
        ARUint8* pVideoData = arVideoGetImage();

        if (pVideoData != 0)
        {
            static_cast<IplImage*>(m_OriginalColorFrameBGR)->imageData = static_cast<char*>(static_cast<void*>(pVideoData));

#ifdef __APPLE__
            cvCvtColor(m_OriginalColorFrameBGR, m_OriginalColorFrameRGB, CV_BGRA2RGB);
#else
            cvCvtColor(m_OriginalColorFrameBGR, m_OriginalColorFrameRGB, CV_BGR2RGB);
#endif

            // -----------------------------------------------------------------------------
            // Convert to output
            // -----------------------------------------------------------------------------
            if (m_FreezeLastFrame == false)
            {
                ConvertOriginalToOutput();
            }
        }
    }

    // -----------------------------------------------------------------------------

    void CWebcamControl::ConvertOriginalToOutput()
    {
        cvResize(m_OriginalColorFrameRGB, m_ConvertedColorFrame);

        Dt::TextureManager::CopyToTexture2D(m_pConvertedFrame, static_cast<void*>(static_cast<IplImage*>(m_ConvertedColorFrame)->imageData));

        Dt::TextureManager::MarkTextureAsDirty(m_pConvertedFrame, Dt::CTextureBase::DirtyData);
    }
} // namespace MR