
#include "base/base_memory.h"

#include "data/data_texture_manager.h"

#include "mr/mr_control_manager.h"
#include "mr/mr_webcam_control.h"

#include <AR/video.h>

#include "opencv2/opencv.hpp"

#define CROP_PERCENTAGE 0.8f
#define IMAGE_EDGE_LENGTH 512
#define IMAGE_SPACE 0
#define INPAINT_RADIUS 0
#define INPAINT_METHOD INPAINT_TELEA

cv::Mat RedImage;
cv::Mat OriginalFrontImage, FrontCroped, FrontLeftPart, FrontRightPart, FrontTopPart, FrontBottomPart;
cv::Mat OriginalBackImage, BackCroped, BackLeftPart, BackRightPart, BackTopPart, BackBottomPart;

cv::Mat CombinedRight, CombinedLeft, CombinedTop, CombinedBottom;

namespace
{
	std::string g_PathToAssets = "../assets/";
} // namespace

namespace MR
{
    CWebcamControl::CWebcamControl()
        : CControl               (CControl::Webcam)
        , m_DeviceNumber         (0)
        , m_OriginalColorFrame   ()
        , m_OriginalColorFrameRGB()
        , m_ConvertedColorFrame  ()
    {
    }

    // -----------------------------------------------------------------------------

    CWebcamControl::~CWebcamControl()
    {
    }

    // -----------------------------------------------------------------------------

    void CWebcamControl::SetDeviceNumber(unsigned int _DeviceNumber)
    {
        m_DeviceNumber = _DeviceNumber;
    }

    // -----------------------------------------------------------------------------

    unsigned int CWebcamControl::GetDeviceNumber() const
    {
        return m_DeviceNumber;
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
        std::stringstream VideoParameter;

#ifdef __APPLE__
        VideoParameter << "-source=" << _rDescriptor.m_DeviceNumber;
#else
        VideoParameter << "-device=WinDS -showDialog -flipV -devNum=" << m_DeviceNumber;
#endif  

        Error = arVideoOpen(VideoParameter.str().c_str());

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

        m_OriginalColorFrame    = cvCreateImage(cvSize(OriginalSizeU, OriginalSizeV), IPL_DEPTH_8U, NumberOfVideoComponents);
        m_OriginalColorFrameRGB = cvCreateImage(cvSize(OriginalSizeU, OriginalSizeV), IPL_DEPTH_8U, 3);
        m_ConvertedColorFrame   = cvCreateImage(cvSize(ConvertedSizeU, ConvertedSizeV), IPL_DEPTH_8U, 3);

        CombinedRight.create(cv::Size(IMAGE_EDGE_LENGTH, IMAGE_EDGE_LENGTH), CV_8UC3);
        CombinedLeft.create(cv::Size(IMAGE_EDGE_LENGTH, IMAGE_EDGE_LENGTH), CV_8UC3);
        CombinedTop.create(cv::Size(IMAGE_EDGE_LENGTH, IMAGE_EDGE_LENGTH), CV_8UC3);
        CombinedBottom.create(cv::Size(IMAGE_EDGE_LENGTH, IMAGE_EDGE_LENGTH), CV_8UC3);
        FrontCroped.create(cv::Size(IMAGE_EDGE_LENGTH, IMAGE_EDGE_LENGTH), CV_8UC3);
        BackCroped.create(cv::Size(IMAGE_EDGE_LENGTH, IMAGE_EDGE_LENGTH), CV_8UC3);

        // -----------------------------------------------------------------------------
        // Setup intern data
        // -----------------------------------------------------------------------------
        Dt::STextureDescriptor TextureDescriptor;

        TextureDescriptor.m_NumberOfPixelsU  = OriginalSizeU;
        TextureDescriptor.m_NumberOfPixelsV  = OriginalSizeV;
        TextureDescriptor.m_NumberOfPixelsW  = 1;
        TextureDescriptor.m_Format           = Dt::CTextureBase::R8G8B8_UBYTE;
        TextureDescriptor.m_Semantic         = Dt::CTextureBase::Diffuse;
        TextureDescriptor.m_pPixels          = static_cast<IplImage*>(m_OriginalColorFrameRGB)->imageData;
        TextureDescriptor.m_pFileName        = 0;
        TextureDescriptor.m_pIdentifier      = "ID_Webcam_RGB_Original_Output";

        m_pOriginalFrame = Dt::TextureManager::CreateTexture2D(TextureDescriptor);

        // -----------------------------------------------------------------------------

        Dt::TextureManager::CopyToTexture2D(m_pConvertedFrame, static_cast<IplImage*>(m_ConvertedColorFrame)->imageData);

        // -----------------------------------------------------------------------------
        // Set data of original video / image
        // -----------------------------------------------------------------------------
        ARUint8* pVideoData = arVideoGetImage();

        for (; pVideoData == 0;)
        {
            pVideoData = arVideoGetImage();
        }

        static_cast<IplImage*>(m_OriginalColorFrame)->imageData = static_cast<char*>(static_cast<void*>(pVideoData));

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
            static_cast<IplImage*>(m_OriginalColorFrame)->imageData = static_cast<char*>(static_cast<void*>(pVideoData));

            // -----------------------------------------------------------------------------
            // Convert to output
            // -----------------------------------------------------------------------------
            ConvertOriginalToOutput();

            // -----------------------------------------------------------------------------
            // Environment approximation
            // -----------------------------------------------------------------------------
            ProcessEnvironmentApproximation();
        }
    }

    // -----------------------------------------------------------------------------

    void CWebcamControl::ConvertOriginalToOutput()
    {
#ifdef __APPLE__
        cvCvtColor(m_OriginalColorFrame, m_OriginalColorFrameRGB, CV_BGRA2RGB);
#else
        cvCvtColor(m_OriginalColorFrame, m_OriginalColorFrameRGB, CV_BGR2RGB);
#endif

        cvResize(m_OriginalColorFrameRGB, m_ConvertedColorFrame);
    }

    // -----------------------------------------------------------------------------

    void CWebcamControl::ProcessEnvironmentApproximation()
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

            PercentualShortedLength = static_cast<int>(static_cast<float>(ShortedLength) * CROP_PERCENTAGE);

            unsigned int X = (LengthX - PercentualShortedLength) / 2;
            unsigned int Y = (LengthY - PercentualShortedLength) / 2;

            Rect CroppedRectangel(Y, X, PercentualShortedLength, PercentualShortedLength);

            _rCroppedImage = _rOriginal(CroppedRectangel);

            resize(_rCroppedImage, _rCroppedImage, Size(IMAGE_EDGE_LENGTH, IMAGE_EDGE_LENGTH));

            // -----------------------------------------------------------------------------

            _rLeftPart   = Mat::zeros(IMAGE_EDGE_LENGTH, IMAGE_EDGE_LENGTH, _rOriginal.type());
            _rRightPart  = Mat::zeros(IMAGE_EDGE_LENGTH, IMAGE_EDGE_LENGTH, _rOriginal.type());
            _rTopPart    = Mat::zeros(IMAGE_EDGE_LENGTH, IMAGE_EDGE_LENGTH, _rOriginal.type());
            _rBottomPart = Mat::zeros(IMAGE_EDGE_LENGTH, IMAGE_EDGE_LENGTH, _rOriginal.type());

            // -----------------------------------------------------------------------------

            Point2f MaskPoints[4];
            Point2f DestPoints[4];
            Mat     WarpMat;

            DestPoints[0] = Point2f(static_cast<float>(0)                    , static_cast<float>(0));
            DestPoints[1] = Point2f(static_cast<float>(0)                    , static_cast<float>(IMAGE_EDGE_LENGTH - 1));
            DestPoints[2] = Point2f(static_cast<float>(IMAGE_EDGE_LENGTH - 1), static_cast<float>(IMAGE_EDGE_LENGTH - 1));
            DestPoints[3] = Point2f(static_cast<float>(IMAGE_EDGE_LENGTH - 1), static_cast<float>(0));

            // -----------------------------------------------------------------------------

            MaskPoints[0] = Point2f(static_cast<float>(0)    , static_cast<float>(0));
            MaskPoints[1] = Point2f(static_cast<float>(0)    , static_cast<float>(LengthX - 1));
            MaskPoints[2] = Point2f(static_cast<float>(Y - 1), static_cast<float>(X + PercentualShortedLength - 1));
            MaskPoints[3] = Point2f(static_cast<float>(Y - 1), static_cast<float>(X - 1));

            WarpMat = getPerspectiveTransform(MaskPoints, DestPoints);

            warpPerspective(_rOriginal, _rLeftPart, WarpMat, _rLeftPart.size());

            // -----------------------------------------------------------------------------

            MaskPoints[0] = Point2f(static_cast<float>(Y + PercentualShortedLength - 1), static_cast<float>(X - 1));
            MaskPoints[1] = Point2f(static_cast<float>(Y + PercentualShortedLength - 1), static_cast<float>(X + PercentualShortedLength - 1));
            MaskPoints[2] = Point2f(static_cast<float>(LengthY - 1)                    , static_cast<float>(LengthX - 1));
            MaskPoints[3] = Point2f(static_cast<float>(LengthY - 1)                    , static_cast<float>(0));

            WarpMat = getPerspectiveTransform(MaskPoints, DestPoints);

            warpPerspective(_rOriginal, _rRightPart, WarpMat, _rRightPart.size());

            // -----------------------------------------------------------------------------

            MaskPoints[0] = Point2f(static_cast<float>(0)                              , static_cast<float>(0));
            MaskPoints[1] = Point2f(static_cast<float>(Y - 1)                          , static_cast<float>(X - 1));
            MaskPoints[2] = Point2f(static_cast<float>(Y + PercentualShortedLength - 1), static_cast<float>(X - 1));
            MaskPoints[3] = Point2f(static_cast<float>(LengthY - 1)                    , static_cast<float>(0));

            WarpMat = getPerspectiveTransform(MaskPoints, DestPoints);

            warpPerspective(_rOriginal, _rTopPart, WarpMat, _rTopPart.size());

            // -----------------------------------------------------------------------------

            MaskPoints[0] = Point2f(static_cast<float>(Y - 1)                          , static_cast<float>(X + PercentualShortedLength - 1));
            MaskPoints[1] = Point2f(static_cast<float>(0)                              , static_cast<float>(LengthX - 1));
            MaskPoints[2] = Point2f(static_cast<float>(LengthY - 1)                    , static_cast<float>(LengthX - 1));
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

            MaskPoints[0] = Point2f(static_cast<float>(0)                , static_cast<float>(0));
            MaskPoints[1] = Point2f(static_cast<float>(0)                , static_cast<float>(IMAGE_EDGE_LENGTH));
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

            DestPointsOne[0] = Point2f(static_cast<float>(0)                                  , static_cast<float>(0));
            DestPointsOne[1] = Point2f(static_cast<float>(0)                                  , static_cast<float>(IMAGE_EDGE_LENGTH));
            DestPointsOne[2] = Point2f(static_cast<float>(IMAGE_EDGE_LENGTH / 2 - IMAGE_SPACE), static_cast<float>(IMAGE_EDGE_LENGTH));

            DestPointsTwo[0] = Point2f(static_cast<float>(IMAGE_EDGE_LENGTH / 2 + IMAGE_SPACE), static_cast<float>(0));
            DestPointsTwo[1] = Point2f(static_cast<float>(IMAGE_EDGE_LENGTH / 2 + IMAGE_SPACE), static_cast<float>(IMAGE_EDGE_LENGTH));
            DestPointsTwo[2] = Point2f(static_cast<float>(IMAGE_EDGE_LENGTH)                  , static_cast<float>(IMAGE_EDGE_LENGTH));

            Mat Combination = CombineFaces(_rOne, _rTwo, DestPointsOne, DestPointsTwo);

            // -----------------------------------------------------------------------------

            Mat Mask   = Mat::zeros(IMAGE_EDGE_LENGTH, IMAGE_EDGE_LENGTH, CV_8U);
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
        };

        // -----------------------------------------------------------------------------

        auto CombineLeftFaces = [&](const Mat& _rOne, const Mat& _rTwo)->cv::Mat
        {
            Point2f DestPointsOne[3];
            Point2f DestPointsTwo[3];

            DestPointsOne[0] = Point2f(static_cast<float>(IMAGE_EDGE_LENGTH / 2 + IMAGE_SPACE), static_cast<float>(0));
            DestPointsOne[1] = Point2f(static_cast<float>(IMAGE_EDGE_LENGTH / 2 + IMAGE_SPACE), static_cast<float>(IMAGE_EDGE_LENGTH));
            DestPointsOne[2] = Point2f(static_cast<float>(IMAGE_EDGE_LENGTH)                  , static_cast<float>(IMAGE_EDGE_LENGTH));

            DestPointsTwo[0] = Point2f(static_cast<float>(0)                                  , static_cast<float>(0));
            DestPointsTwo[1] = Point2f(static_cast<float>(0)                                  , static_cast<float>(IMAGE_EDGE_LENGTH));
            DestPointsTwo[2] = Point2f(static_cast<float>(IMAGE_EDGE_LENGTH / 2 - IMAGE_SPACE), static_cast<float>(IMAGE_EDGE_LENGTH));

            Mat Combination = CombineFaces(_rOne, _rTwo, DestPointsOne, DestPointsTwo);

            // -----------------------------------------------------------------------------

            Mat Mask   = Mat::zeros(IMAGE_EDGE_LENGTH, IMAGE_EDGE_LENGTH, CV_8U);
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
        };

        // -----------------------------------------------------------------------------

        auto CombineTopFaces = [&](const Mat& _rOne, const Mat& _rTwo)->cv::Mat
        {
            Point2f DestPointsOne[3];
            Point2f DestPointsTwo[3];

            DestPointsOne[0] = Point2f(static_cast<float>(0)                , static_cast<float>(IMAGE_EDGE_LENGTH / 2 + IMAGE_SPACE));
            DestPointsOne[1] = Point2f(static_cast<float>(0)                , static_cast<float>(IMAGE_EDGE_LENGTH));
            DestPointsOne[2] = Point2f(static_cast<float>(IMAGE_EDGE_LENGTH), static_cast<float>(IMAGE_EDGE_LENGTH));

            DestPointsTwo[0] = Point2f(static_cast<float>(0)                , static_cast<float>(0));
            DestPointsTwo[1] = Point2f(static_cast<float>(0)                , static_cast<float>(IMAGE_EDGE_LENGTH / 2 - IMAGE_SPACE));
            DestPointsTwo[2] = Point2f(static_cast<float>(IMAGE_EDGE_LENGTH), static_cast<float>(IMAGE_EDGE_LENGTH / 2 - IMAGE_SPACE));

            Mat Combination = CombineFaces(_rOne, _rTwo, DestPointsOne, DestPointsTwo);

            // -----------------------------------------------------------------------------

            Mat Mask   = Mat::zeros(IMAGE_EDGE_LENGTH, IMAGE_EDGE_LENGTH, CV_8U);
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
        };

        // -----------------------------------------------------------------------------

        auto CombineBottomFaces = [&](const Mat& _rOne, const Mat& _rTwo)->cv::Mat
        {
            Point2f DestPointsOne[3];
            Point2f DestPointsTwo[3];

            DestPointsOne[0] = Point2f(static_cast<float>(0)                , static_cast<float>(0));
            DestPointsOne[1] = Point2f(static_cast<float>(0)                , static_cast<float>(IMAGE_EDGE_LENGTH / 2 - IMAGE_SPACE));
            DestPointsOne[2] = Point2f(static_cast<float>(IMAGE_EDGE_LENGTH), static_cast<float>(IMAGE_EDGE_LENGTH / 2 - IMAGE_SPACE));

            DestPointsTwo[0] = Point2f(static_cast<float>(0)                , static_cast<float>(IMAGE_EDGE_LENGTH / 2 + IMAGE_SPACE));
            DestPointsTwo[1] = Point2f(static_cast<float>(0)                , static_cast<float>(IMAGE_EDGE_LENGTH));
            DestPointsTwo[2] = Point2f(static_cast<float>(IMAGE_EDGE_LENGTH), static_cast<float>(IMAGE_EDGE_LENGTH));

            Mat Combination = CombineFaces(_rOne, _rTwo, DestPointsOne, DestPointsTwo);

            // -----------------------------------------------------------------------------

            Mat Mask   = Mat::zeros(IMAGE_EDGE_LENGTH, IMAGE_EDGE_LENGTH, CV_8U);
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
        };

        // -----------------------------------------------------------------------------

        auto BlurEdges = [&](const Mat& _Input, Mat& _Output)
        {
            Mat Test;
            Mat UnBlurred;
            Mat Blurred;

            UnBlurred = _Input;

            GaussianBlur(_Input, Blurred, Size(33, 33), 5.0);

            Canny(_Input, Test, 5, 40);

            GaussianBlur(Test, Test, Size(33, 33), 15.0);

            Mat NotTest;

            bitwise_not(Test, NotTest);

            cvtColor(NotTest, NotTest, CV_GRAY2RGB);

            cvtColor(Test, Test, CV_GRAY2RGB);

            add(Blurred.mul(NotTest / 255), _Input.mul(Test / 255), _Output);
        };

        IplImage* pConvertedColorFrame = static_cast<IplImage*>(m_ConvertedColorFrame);

        OriginalFrontImage = cvarrToMat(pConvertedColorFrame, true);

        flip(OriginalFrontImage, OriginalBackImage, 1);

        // -----------------------------------------------------------------------------
        // Crop front image
        // -----------------------------------------------------------------------------
        CropImage(OriginalFrontImage, FrontCroped, FrontLeftPart, FrontRightPart, FrontTopPart, FrontBottomPart);
        CropImage(OriginalBackImage, BackCroped, BackLeftPart, BackRightPart, BackTopPart, BackBottomPart);

        // -----------------------------------------------------------------------------
        // Flip back images because of negative direction on back face
        // -----------------------------------------------------------------------------
        flip(BackTopPart, BackTopPart, -1);
        flip(BackBottomPart, BackBottomPart, -1);

        // -----------------------------------------------------------------------------
        // Fill Images
        // -----------------------------------------------------------------------------
        CombinedRight  = CombineRightFaces(FrontRightPart, BackLeftPart);
        CombinedLeft   = CombineLeftFaces(FrontLeftPart, BackRightPart);
        CombinedTop    = CombineTopFaces(FrontTopPart, BackTopPart);
        CombinedBottom = CombineBottomFaces(FrontBottomPart, BackBottomPart);

        Dt::TextureManager::CopyToTexture2D(m_pCubemap->GetFace(Dt::CTextureCube::Right),  CombinedRight.data);
        Dt::TextureManager::CopyToTexture2D(m_pCubemap->GetFace(Dt::CTextureCube::Left),   CombinedLeft.data);
        Dt::TextureManager::CopyToTexture2D(m_pCubemap->GetFace(Dt::CTextureCube::Top),    CombinedTop.data);
        Dt::TextureManager::CopyToTexture2D(m_pCubemap->GetFace(Dt::CTextureCube::Bottom), CombinedBottom.data);
        Dt::TextureManager::CopyToTexture2D(m_pCubemap->GetFace(Dt::CTextureCube::Front),  FrontCroped.data);
        Dt::TextureManager::CopyToTexture2D(m_pCubemap->GetFace(Dt::CTextureCube::Back),   BackCroped.data);
    }
} // namespace MR