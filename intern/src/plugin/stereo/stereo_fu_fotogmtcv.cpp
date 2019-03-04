#include "plugin\stereo\stereo_precompiled.h"
#include "plugin\stereo\stereo_fu_fotogmtcv.h"


namespace Stereo
{
    Fu_FotoGmtCV::Fu_FotoGmtCV()
    {
    }
    
    Fu_FotoGmtCV::Fu_FotoGmtCV(const std::vector<char>& Img_Input, int ImgW, int ImgH)
    {
        // Latter apply switch for different types of images
        cv::Mat Img_Input_cv(cv::Size(ImgW, ImgH), CV_8UC4); // 2D Matrix(x*y) with (8-bit unsigned character) + (4 bands)
        memcpy(Img_Input_cv.data, Img_Input.data(), Img_Input.size());
        cv::cvtColor(Img_Input_cv, Img_Input_cv, cv::COLOR_BGRA2RGBA); // Transform image from BGRA (default of OpenCV) to RGB
        FuImg = Img_Input_cv; // 1D vector stores 2D image in row-oriented
    }
    Fu_FotoGmtCV::~Fu_FotoGmtCV()
    {
    }

    void Fu_FotoGmtCV::setPmtx(glm::mat4x3 P) // P is the transform from Mapping frame to Image frame
    {
        P_mtx = P;
    }

    void Fu_FotoGmtCV::ShowImg()
    {
        cv::imshow("Image", FuImg);
    }

} // Stereo
