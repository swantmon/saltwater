
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

#define CROP_PERCENTAGE 0.8f
#define IMAGE_EDGE_LENGTH 512
#define IMAGE_SPACE 0
#define USE_INPAINTING 0
#define INPAINT_RADIUS 0
#define INPAINT_METHOD INPAINT_TELEA

cv::Mat g_OriginalFrontImage, g_FrontCroped, g_FrontLeftPart, g_FrontRightPart, g_FrontTopPart, g_FrontBottomPart;
cv::Mat g_OriginalBackImage, g_BackCroped, g_BackLeftPart, g_BackRightPart, g_BackTopPart, g_BackBottomPart;

cv::Mat g_CombinedRight, g_CombinedLeft, g_CombinedTop, g_CombinedBottom;

IKinectSensor*     g_pSensor;
IColorFrameReader* g_pColorFrameReader;
IDepthFrameReader* g_pDepthFrameReader;
ICoordinateMapper* g_pCoordinateMapper;
unsigned char      g_pColorFrameDataRAW[COLOR_WIDTH * COLOR_HEIGHT * 4];
float              g_pDepthFrameDataRAW[COLOR_WIDTH * COLOR_HEIGHT];
cv::Mat            DepthFC1CropTemp = cv::Mat(720, 1280, CV_32FC1, 0);

cv::Mat DepthFC1     = cv::Mat(1080, 1920, CV_32FC1, &g_pDepthFrameDataRAW);
cv::Mat ColorUC4     = cv::Mat(1080, 1920, CV_8UC4, &g_pColorFrameDataRAW);
cv::Mat ColorUC3     = cv::Mat(1080, 1920, CV_8UC3, 0);
cv::Mat ColorUC1Crop = cv::Mat(720, 1280, CV_8UC1, 0);
cv::Mat DepthFC1Crop = cv::Mat(720, 1280, CV_32FC1, 0);
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
        return static_cast<void*>(static_cast<Base::Ptr>(DepthFC1.data));
    }

    // -----------------------------------------------------------------------------

    void* CKinectControl::GetConvertedDepthFrame() const
    {
        return static_cast<void*>(static_cast<Base::Ptr>(DepthFC1Crop.data));
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
        // Setup OpenCV images for further image processing
        // -----------------------------------------------------------------------------
        g_CombinedRight.create(cv::Size(IMAGE_EDGE_LENGTH, IMAGE_EDGE_LENGTH), CV_8UC3);
        g_CombinedLeft.create(cv::Size(IMAGE_EDGE_LENGTH, IMAGE_EDGE_LENGTH), CV_8UC3);
        g_CombinedTop.create(cv::Size(IMAGE_EDGE_LENGTH, IMAGE_EDGE_LENGTH), CV_8UC3);
        g_CombinedBottom.create(cv::Size(IMAGE_EDGE_LENGTH, IMAGE_EDGE_LENGTH), CV_8UC3);
        g_FrontCroped.create(cv::Size(IMAGE_EDGE_LENGTH, IMAGE_EDGE_LENGTH), CV_8UC3);
        g_BackCroped.create(cv::Size(IMAGE_EDGE_LENGTH, IMAGE_EDGE_LENGTH), CV_8UC3);

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

        pDepthFrameSource->OpenReader(&g_pDepthFrameReader);

        if (pDepthFrameSource != 0)
        {
            pDepthFrameSource->Release();

            pDepthFrameSource = 0;
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
        TextureDescriptor.m_pPixels          = 0;
        TextureDescriptor.m_pFileName        = 0;
        TextureDescriptor.m_pIdentifier      = "ID_Webcam_RGB_Original_Output";

        m_pOriginalFrame = Dt::TextureManager::CreateTexture2D(TextureDescriptor);
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

        if (g_pDepthFrameReader == 0)
        {
            g_pDepthFrameReader->Release();

            g_pDepthFrameReader = 0;
        }

        // -----------------------------------------------------------------------------
        // Shutdown device
        // -----------------------------------------------------------------------------
        if (!g_pSensor)
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
            unsigned int Capacity;
            unsigned short* pBuffer;

            pDepthFrame->AccessUnderlyingBuffer(&Capacity, &pBuffer);

            static CameraSpacePoint CameraSpaceOfColorFrame[COLOR_WIDTH * COLOR_HEIGHT];

            g_pCoordinateMapper->MapColorFrameToCameraSpace(
                DEPTH_WIDTH * DEPTH_HEIGHT, pBuffer,
                COLOR_WIDTH * COLOR_HEIGHT, CameraSpaceOfColorFrame);

            assert(Capacity == DEPTH_HEIGHT * DEPTH_WIDTH);

            for (unsigned int IndexOfColorValue = 0; IndexOfColorValue < COLOR_WIDTH * COLOR_HEIGHT; ++IndexOfColorValue)
            {
                // -----------------------------------------------------------------------------
                // Depth in millimeters
                // -----------------------------------------------------------------------------
                CameraSpacePoint ColorCameraSpacePoint = CameraSpaceOfColorFrame[IndexOfColorValue];

                // -----------------------------------------------------------------------------
                // Index of destination
                // -----------------------------------------------------------------------------
                g_pDepthFrameDataRAW[IndexOfColorValue] = 0.0f;

                if (!std::isinf(fabsf(ColorCameraSpacePoint.Z)))
                {
                    g_pDepthFrameDataRAW[IndexOfColorValue] = ColorCameraSpacePoint.Z;
                }
            }
        }

        if (pDepthFrame)
        {
            pDepthFrame->Release();

            pDepthFrame = 0;
        }

        // -----------------------------------------------------------------------------
        // Prepare OpenCV data
        // -----------------------------------------------------------------------------
        DepthFC1 = cv::Mat(1080, 1920, CV_32FC1, &g_pDepthFrameDataRAW);
        ColorUC4 = cv::Mat(1080, 1920, CV_8UC4, &g_pColorFrameDataRAW);

        //cv::flip(DepthFC1, DepthFC1, 1);
        //cv::flip(ColorUC4, ColorUC4, 1);

        cv::cvtColor(ColorUC4, ColorUC3, CV_RGBA2RGB);

        ColorUC3(cv::Rect(320, 180, 1280, 720)).copyTo(ColorUC3Crop);
        DepthFC1(cv::Rect(320, 180, 1280, 720)).copyTo(DepthFC1Crop);

        cv::flip(DepthFC1Crop, DepthFC1Crop, 1);
        cv::flip(ColorUC3Crop, ColorUC3Crop, 1);
        cv::flip(ColorUC3    , ColorUC3    , 1);

        // -----------------------------------------------------------------------------
        // Copy final result to the textures
        // -----------------------------------------------------------------------------
        Dt::TextureManager::CopyToTexture2D(m_pOriginalFrame, static_cast<void*>(static_cast<Base::Ptr>(ColorUC3.data)));

        Dt::TextureManager::CopyToTexture2D(m_pConvertedFrame, static_cast<void*>(static_cast<Base::Ptr>(ColorUC3Crop.data)));

        // -----------------------------------------------------------------------------
        // Environment approximation
        // -----------------------------------------------------------------------------
        ProcessEnvironmentApproximation();
    }

	void CKinectControl::ProcessEnvironmentApproximation()
	{
		using namespace cv;

		auto CropImage = [&](const Mat& _rOriginal, Mat& _rCroppedImage, Mat& _rLeftPart, Mat& _rRightPart, Mat& _rTopPart, Mat& _rBottomPart)
		{
			int LengthX;
			int LengthY;
			int ShortedLength;
			int PercentualShortedLength;

			LengthX = _rOriginal.size[0];
			LengthY = _rOriginal.size[1];

			ShortedLength = LengthX < LengthY ? LengthX : LengthY;

			PercentualShortedLength = static_cast<int>(static_cast<float>(ShortedLength)* CROP_PERCENTAGE);

			unsigned int X = (LengthX - PercentualShortedLength) / 2;
			unsigned int Y = (LengthY - PercentualShortedLength) / 2;

			Rect CroppedRectangel(Y, X, PercentualShortedLength, PercentualShortedLength);

			_rCroppedImage = _rOriginal(CroppedRectangel);

			resize(_rCroppedImage, _rCroppedImage, Size(IMAGE_EDGE_LENGTH, IMAGE_EDGE_LENGTH));

			// -----------------------------------------------------------------------------

			_rLeftPart = Mat::zeros(IMAGE_EDGE_LENGTH, IMAGE_EDGE_LENGTH, _rOriginal.type());
			_rRightPart = Mat::zeros(IMAGE_EDGE_LENGTH, IMAGE_EDGE_LENGTH, _rOriginal.type());
			_rTopPart = Mat::zeros(IMAGE_EDGE_LENGTH, IMAGE_EDGE_LENGTH, _rOriginal.type());
			_rBottomPart = Mat::zeros(IMAGE_EDGE_LENGTH, IMAGE_EDGE_LENGTH, _rOriginal.type());

			// -----------------------------------------------------------------------------

			Point2f MaskPoints[4];
			Point2f DestPoints[4];
			Mat     WarpMat;

			DestPoints[0] = Point2f(static_cast<float>(0), static_cast<float>(0));
			DestPoints[1] = Point2f(static_cast<float>(0), static_cast<float>(IMAGE_EDGE_LENGTH - 1));
			DestPoints[2] = Point2f(static_cast<float>(IMAGE_EDGE_LENGTH - 1), static_cast<float>(IMAGE_EDGE_LENGTH - 1));
			DestPoints[3] = Point2f(static_cast<float>(IMAGE_EDGE_LENGTH - 1), static_cast<float>(0));

			// -----------------------------------------------------------------------------

			MaskPoints[0] = Point2f(static_cast<float>(0), static_cast<float>(0));
			MaskPoints[1] = Point2f(static_cast<float>(0), static_cast<float>(LengthX - 1));
			MaskPoints[2] = Point2f(static_cast<float>(Y - 1), static_cast<float>(X + PercentualShortedLength - 1));
			MaskPoints[3] = Point2f(static_cast<float>(Y - 1), static_cast<float>(X - 1));

			WarpMat = getPerspectiveTransform(MaskPoints, DestPoints);

			warpPerspective(_rOriginal, _rLeftPart, WarpMat, _rLeftPart.size());

			// -----------------------------------------------------------------------------

			MaskPoints[0] = Point2f(static_cast<float>(Y + PercentualShortedLength - 1), static_cast<float>(X - 1));
			MaskPoints[1] = Point2f(static_cast<float>(Y + PercentualShortedLength - 1), static_cast<float>(X + PercentualShortedLength - 1));
			MaskPoints[2] = Point2f(static_cast<float>(LengthY - 1), static_cast<float>(LengthX - 1));
			MaskPoints[3] = Point2f(static_cast<float>(LengthY - 1), static_cast<float>(0));

			WarpMat = getPerspectiveTransform(MaskPoints, DestPoints);

			warpPerspective(_rOriginal, _rRightPart, WarpMat, _rRightPart.size());

			// -----------------------------------------------------------------------------

			MaskPoints[0] = Point2f(static_cast<float>(0), static_cast<float>(0));
			MaskPoints[1] = Point2f(static_cast<float>(Y - 1), static_cast<float>(X - 1));
			MaskPoints[2] = Point2f(static_cast<float>(Y + PercentualShortedLength - 1), static_cast<float>(X - 1));
			MaskPoints[3] = Point2f(static_cast<float>(LengthY - 1), static_cast<float>(0));

			WarpMat = getPerspectiveTransform(MaskPoints, DestPoints);

			warpPerspective(_rOriginal, _rTopPart, WarpMat, _rTopPart.size());

			// -----------------------------------------------------------------------------

			MaskPoints[0] = Point2f(static_cast<float>(Y - 1), static_cast<float>(X + PercentualShortedLength - 1));
			MaskPoints[1] = Point2f(static_cast<float>(0), static_cast<float>(LengthX - 1));
			MaskPoints[2] = Point2f(static_cast<float>(LengthY - 1), static_cast<float>(LengthX - 1));
			MaskPoints[3] = Point2f(static_cast<float>(Y + PercentualShortedLength - 1), static_cast<float>(X + PercentualShortedLength - 1));

			WarpMat = getPerspectiveTransform(MaskPoints, DestPoints);

			warpPerspective(_rOriginal, _rBottomPart, WarpMat, _rBottomPart.size());
		};

		// -----------------------------------------------------------------------------

		auto CombineFaces = [&](const Mat& _rOne, const Mat& _rTwo, const Point2f* _pDestinationOne, const Point2f* _pDestinationTwo)->cv::Mat
		{
			Mat CombinedOne, CombinedTwo;

			CombinedOne = Mat::zeros(IMAGE_EDGE_LENGTH, IMAGE_EDGE_LENGTH, _rOne.type());
			CombinedTwo = Mat::zeros(IMAGE_EDGE_LENGTH, IMAGE_EDGE_LENGTH, _rOne.type());

			// -----------------------------------------------------------------------------

			Point2f MaskPoints[3];

			MaskPoints[0] = Point2f(static_cast<float>(0), static_cast<float>(0));
			MaskPoints[1] = Point2f(static_cast<float>(0), static_cast<float>(IMAGE_EDGE_LENGTH));
			MaskPoints[2] = Point2f(static_cast<float>(IMAGE_EDGE_LENGTH), static_cast<float>(IMAGE_EDGE_LENGTH));

			Mat WarpMat;

			// -----------------------------------------------------------------------------

			WarpMat = getAffineTransform(MaskPoints, _pDestinationOne);

			warpAffine(_rOne, CombinedOne, WarpMat, CombinedOne.size());

			// -----------------------------------------------------------------------------

			WarpMat = getAffineTransform(MaskPoints, _pDestinationTwo);

			warpAffine(_rTwo, CombinedTwo, WarpMat, CombinedTwo.size());

			return CombinedOne + CombinedTwo;
		};

		// -----------------------------------------------------------------------------

		auto CombineRightFaces = [&](const Mat& _rOne, const Mat& _rTwo)->cv::Mat
		{
			Point2f DestPointsOne[3];
			Point2f DestPointsTwo[3];

			DestPointsOne[0] = Point2f(static_cast<float>(0), static_cast<float>(0));
			DestPointsOne[1] = Point2f(static_cast<float>(0), static_cast<float>(IMAGE_EDGE_LENGTH));
			DestPointsOne[2] = Point2f(static_cast<float>(IMAGE_EDGE_LENGTH / 2 - IMAGE_SPACE), static_cast<float>(IMAGE_EDGE_LENGTH));

			DestPointsTwo[0] = Point2f(static_cast<float>(IMAGE_EDGE_LENGTH / 2 + IMAGE_SPACE), static_cast<float>(0));
			DestPointsTwo[1] = Point2f(static_cast<float>(IMAGE_EDGE_LENGTH / 2 + IMAGE_SPACE), static_cast<float>(IMAGE_EDGE_LENGTH));
			DestPointsTwo[2] = Point2f(static_cast<float>(IMAGE_EDGE_LENGTH), static_cast<float>(IMAGE_EDGE_LENGTH));

			Mat Combination = CombineFaces(_rOne, _rTwo, DestPointsOne, DestPointsTwo);

#if USE_INPAINTING == 0
			return Combination;
#else
			Mat Mask = Mat::zeros(IMAGE_EDGE_LENGTH, IMAGE_EDGE_LENGTH, CV_8U);
			Mat Result = Mat::zeros(IMAGE_EDGE_LENGTH, IMAGE_EDGE_LENGTH, Combination.type());

			Point MaskPoints[3];

			MaskPoints[0] = Point(IMAGE_EDGE_LENGTH / 2 - IMAGE_SPACE * 2, 0);
			MaskPoints[1] = Point(IMAGE_EDGE_LENGTH / 2 - IMAGE_SPACE * 2, IMAGE_EDGE_LENGTH);
			MaskPoints[2] = Point(IMAGE_EDGE_LENGTH / 2 + IMAGE_SPACE * 2, 0);

			Scalar Color = Scalar(255, 255, 255);

			fillConvexPoly(Mask, MaskPoints, 3, Color);

			MaskPoints[0] = Point(IMAGE_EDGE_LENGTH / 2 - IMAGE_SPACE * 2, IMAGE_EDGE_LENGTH);
			MaskPoints[1] = Point(IMAGE_EDGE_LENGTH / 2 + IMAGE_SPACE * 2, 0);
			MaskPoints[2] = Point(IMAGE_EDGE_LENGTH / 2 + IMAGE_SPACE * 2, IMAGE_EDGE_LENGTH);

			fillConvexPoly(Mask, MaskPoints, 3, Color);

			// -----------------------------------------------------------------------------

			inpaint(Combination, Mask, Result, INPAINT_RADIUS, INPAINT_METHOD);

			// -----------------------------------------------------------------------------

			return Result;
#endif
		};

		// -----------------------------------------------------------------------------

		auto CombineLeftFaces = [&](const Mat& _rOne, const Mat& _rTwo)->cv::Mat
		{
			Point2f DestPointsOne[3];
			Point2f DestPointsTwo[3];

			DestPointsOne[0] = Point2f(static_cast<float>(IMAGE_EDGE_LENGTH / 2 + IMAGE_SPACE), static_cast<float>(0));
			DestPointsOne[1] = Point2f(static_cast<float>(IMAGE_EDGE_LENGTH / 2 + IMAGE_SPACE), static_cast<float>(IMAGE_EDGE_LENGTH));
			DestPointsOne[2] = Point2f(static_cast<float>(IMAGE_EDGE_LENGTH), static_cast<float>(IMAGE_EDGE_LENGTH));

			DestPointsTwo[0] = Point2f(static_cast<float>(0), static_cast<float>(0));
			DestPointsTwo[1] = Point2f(static_cast<float>(0), static_cast<float>(IMAGE_EDGE_LENGTH));
			DestPointsTwo[2] = Point2f(static_cast<float>(IMAGE_EDGE_LENGTH / 2 - IMAGE_SPACE), static_cast<float>(IMAGE_EDGE_LENGTH));

			Mat Combination = CombineFaces(_rOne, _rTwo, DestPointsOne, DestPointsTwo);

#if USE_INPAINTING == 0
			return Combination;
#else
			Mat Mask = Mat::zeros(IMAGE_EDGE_LENGTH, IMAGE_EDGE_LENGTH, CV_8U);
			Mat Result = Mat::zeros(IMAGE_EDGE_LENGTH, IMAGE_EDGE_LENGTH, Combination.type());

			Point MaskPoints[3];

			MaskPoints[0] = Point(IMAGE_EDGE_LENGTH / 2 - IMAGE_SPACE * 2, 0);
			MaskPoints[1] = Point(IMAGE_EDGE_LENGTH / 2 - IMAGE_SPACE * 2, IMAGE_EDGE_LENGTH);
			MaskPoints[2] = Point(IMAGE_EDGE_LENGTH / 2 + IMAGE_SPACE * 2, 0);

			Scalar Color = Scalar(255, 255, 255);

			fillConvexPoly(Mask, MaskPoints, 3, Color);

			MaskPoints[0] = Point(IMAGE_EDGE_LENGTH / 2 - IMAGE_SPACE * 2, IMAGE_EDGE_LENGTH);
			MaskPoints[1] = Point(IMAGE_EDGE_LENGTH / 2 + IMAGE_SPACE * 2, 0);
			MaskPoints[2] = Point(IMAGE_EDGE_LENGTH / 2 + IMAGE_SPACE * 2, IMAGE_EDGE_LENGTH);

			fillConvexPoly(Mask, MaskPoints, 3, Color);

			// -----------------------------------------------------------------------------

			inpaint(Combination, Mask, Result, INPAINT_RADIUS, INPAINT_METHOD);

			// -----------------------------------------------------------------------------

			return Result;
#endif
		};

		// -----------------------------------------------------------------------------

		auto CombineTopFaces = [&](const Mat& _rOne, const Mat& _rTwo)->cv::Mat
		{
			Point2f DestPointsOne[3];
			Point2f DestPointsTwo[3];

			DestPointsOne[0] = Point2f(static_cast<float>(0), static_cast<float>(IMAGE_EDGE_LENGTH / 2 + IMAGE_SPACE));
			DestPointsOne[1] = Point2f(static_cast<float>(0), static_cast<float>(IMAGE_EDGE_LENGTH));
			DestPointsOne[2] = Point2f(static_cast<float>(IMAGE_EDGE_LENGTH), static_cast<float>(IMAGE_EDGE_LENGTH));

			DestPointsTwo[0] = Point2f(static_cast<float>(0), static_cast<float>(0));
			DestPointsTwo[1] = Point2f(static_cast<float>(0), static_cast<float>(IMAGE_EDGE_LENGTH / 2 - IMAGE_SPACE));
			DestPointsTwo[2] = Point2f(static_cast<float>(IMAGE_EDGE_LENGTH), static_cast<float>(IMAGE_EDGE_LENGTH / 2 - IMAGE_SPACE));

			Mat Combination = CombineFaces(_rOne, _rTwo, DestPointsOne, DestPointsTwo);

#if USE_INPAINTING == 0
			return Combination;
#else
			Mat Mask = Mat::zeros(IMAGE_EDGE_LENGTH, IMAGE_EDGE_LENGTH, CV_8U);
			Mat Result = Mat::zeros(IMAGE_EDGE_LENGTH, IMAGE_EDGE_LENGTH, Combination.type());

			Point MaskPoints[3];

			MaskPoints[0] = Point(0, IMAGE_EDGE_LENGTH / 2 - IMAGE_SPACE * 2);
			MaskPoints[1] = Point(0, IMAGE_EDGE_LENGTH / 2 + IMAGE_SPACE * 2);
			MaskPoints[2] = Point(IMAGE_EDGE_LENGTH, IMAGE_EDGE_LENGTH / 2 + IMAGE_SPACE * 2);

			Scalar Color = Scalar(255, 255, 255);

			fillConvexPoly(Mask, MaskPoints, 3, Color);

			MaskPoints[0] = Point(0, IMAGE_EDGE_LENGTH / 2 - IMAGE_SPACE * 2);
			MaskPoints[1] = Point(IMAGE_EDGE_LENGTH, IMAGE_EDGE_LENGTH / 2 + IMAGE_SPACE * 2);
			MaskPoints[2] = Point(IMAGE_EDGE_LENGTH, IMAGE_EDGE_LENGTH / 2 - IMAGE_SPACE * 2);

			fillConvexPoly(Mask, MaskPoints, 3, Color);

			// -----------------------------------------------------------------------------

			inpaint(Combination, Mask, Result, INPAINT_RADIUS, INPAINT_METHOD);

			// -----------------------------------------------------------------------------

			return Result;
#endif
		};

		// -----------------------------------------------------------------------------

		auto CombineBottomFaces = [&](const Mat& _rOne, const Mat& _rTwo)->cv::Mat
		{
			Point2f DestPointsOne[3];
			Point2f DestPointsTwo[3];

			DestPointsOne[0] = Point2f(static_cast<float>(0), static_cast<float>(0));
			DestPointsOne[1] = Point2f(static_cast<float>(0), static_cast<float>(IMAGE_EDGE_LENGTH / 2 - IMAGE_SPACE));
			DestPointsOne[2] = Point2f(static_cast<float>(IMAGE_EDGE_LENGTH), static_cast<float>(IMAGE_EDGE_LENGTH / 2 - IMAGE_SPACE));

			DestPointsTwo[0] = Point2f(static_cast<float>(0), static_cast<float>(IMAGE_EDGE_LENGTH / 2 + IMAGE_SPACE));
			DestPointsTwo[1] = Point2f(static_cast<float>(0), static_cast<float>(IMAGE_EDGE_LENGTH));
			DestPointsTwo[2] = Point2f(static_cast<float>(IMAGE_EDGE_LENGTH), static_cast<float>(IMAGE_EDGE_LENGTH));

			Mat Combination = CombineFaces(_rOne, _rTwo, DestPointsOne, DestPointsTwo);

#if USE_INPAINTING == 0
			return Combination;
#else
			Mat Mask = Mat::zeros(IMAGE_EDGE_LENGTH, IMAGE_EDGE_LENGTH, CV_8U);
			Mat Result = Mat::zeros(IMAGE_EDGE_LENGTH, IMAGE_EDGE_LENGTH, Combination.type());

			Point MaskPoints[3];

			MaskPoints[0] = Point(0, IMAGE_EDGE_LENGTH / 2 - IMAGE_SPACE * 2);
			MaskPoints[1] = Point(0, IMAGE_EDGE_LENGTH / 2 + IMAGE_SPACE * 2);
			MaskPoints[2] = Point(IMAGE_EDGE_LENGTH, IMAGE_EDGE_LENGTH / 2 + IMAGE_SPACE * 2);

			Scalar Color = Scalar(255, 255, 255);

			fillConvexPoly(Mask, MaskPoints, 3, Color);

			MaskPoints[0] = Point(0, IMAGE_EDGE_LENGTH / 2 - IMAGE_SPACE * 2);
			MaskPoints[1] = Point(IMAGE_EDGE_LENGTH, IMAGE_EDGE_LENGTH / 2 + IMAGE_SPACE * 2);
			MaskPoints[2] = Point(IMAGE_EDGE_LENGTH, IMAGE_EDGE_LENGTH / 2 - IMAGE_SPACE * 2);

			fillConvexPoly(Mask, MaskPoints, 3, Color);

			// -----------------------------------------------------------------------------

			inpaint(Combination, Mask, Result, INPAINT_RADIUS, INPAINT_METHOD);

			// -----------------------------------------------------------------------------

			return Result;
#endif
		};

		// -----------------------------------------------------------------------------

		// IplImage* pConvertedColorFrame = static_cast<IplImage*>(.data);

        g_OriginalFrontImage = ColorUC3Crop;// cvarrToMat(pConvertedColorFrame, true);

		flip(g_OriginalFrontImage, g_OriginalBackImage, 1);

		// -----------------------------------------------------------------------------
		// Crop front image
		// -----------------------------------------------------------------------------
		CropImage(g_OriginalFrontImage, g_FrontCroped, g_FrontLeftPart, g_FrontRightPart, g_FrontTopPart, g_FrontBottomPart);
		CropImage(g_OriginalBackImage, g_BackCroped, g_BackLeftPart, g_BackRightPart, g_BackTopPart, g_BackBottomPart);

		// -----------------------------------------------------------------------------
		// Flip back images because of negative direction on back face
		// -----------------------------------------------------------------------------
		flip(g_BackTopPart, g_BackTopPart, -1);
		flip(g_BackBottomPart, g_BackBottomPart, -1);

		// -----------------------------------------------------------------------------
		// Fill Images
		// -----------------------------------------------------------------------------
		g_CombinedRight = CombineRightFaces(g_FrontRightPart, g_BackLeftPart);
		g_CombinedLeft = CombineLeftFaces(g_FrontLeftPart, g_BackRightPart);
		g_CombinedTop = CombineTopFaces(g_FrontTopPart, g_BackTopPart);
		g_CombinedBottom = CombineBottomFaces(g_FrontBottomPart, g_BackBottomPart);

        Dt::TextureManager::CopyToTextureCube(m_pCubemap, Dt::CTextureCube::Right, g_CombinedRight.data);
        Dt::TextureManager::CopyToTextureCube(m_pCubemap, Dt::CTextureCube::Left, g_CombinedLeft.data);
        Dt::TextureManager::CopyToTextureCube(m_pCubemap, Dt::CTextureCube::Top, g_CombinedTop.data);
        Dt::TextureManager::CopyToTextureCube(m_pCubemap, Dt::CTextureCube::Bottom, g_CombinedBottom.data);
        Dt::TextureManager::CopyToTextureCube(m_pCubemap, Dt::CTextureCube::Front, g_FrontCroped.data);
        Dt::TextureManager::CopyToTextureCube(m_pCubemap, Dt::CTextureCube::Back, g_BackCroped.data);
	}
} // namespace MR