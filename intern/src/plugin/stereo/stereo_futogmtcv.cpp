#include "plugin\stereo\stereo_precompiled.h"
#include "plugin\stereo\stereo_futogmtcv.h"


namespace Stereo
{
    //---Constructors & Destructor---
    FutoGmtCV::FutoGmtCV()
    {
    }

    FutoGmtCV::FutoGmtCV(cv::Mat& Img_Input)
    {
        Img = Img_Input;
    }

    FutoGmtCV::FutoGmtCV(const std::vector<char>& Img_Input, int ImgW, int ImgH)
    {
        // Latter apply switch for different types of images
        cv::Mat Img_Input_cv(cv::Size(ImgW, ImgH), CV_8UC4); // 2D Matrix(x*y) with (8-bit unsigned character) + (4 bands)
        memcpy(Img_Input_cv.data, Img_Input.data(), Img_Input.size());
        cv::cvtColor(Img_Input_cv, Img_Input_cv, cv::COLOR_BGRA2RGBA); // Transform image from BGRA (default of OpenCV) to RGB
        Img = Img_Input_cv; // 1D vector stores 2D image in row-oriented
    }
    FutoGmtCV::~FutoGmtCV()
    {
    }

    //---Photogrammetric Computer Vision---
    void FutoGmtCV::cal_PolarRect(cv::Mat& RectImg_Base, cv::Mat& RectImg_Match, const cv::Mat& Img_Match, const cv::Mat F_mtx) // Epipolarization based on Polar Rectification
    {
        oper_PolarRect = PolarRect(Img, Img_Match);

        oper_PolarRect.compute(F_mtx);

        oper_PolarRect.genrt_RectImg();

        oper_PolarRect.get_RectImg(RectImg_Base, RectImg_Match);
    }

    //---Orientation & Transformation---
    void FutoGmtCV::cal_F_mtx(const cv::Mat& P_ImgM, cv::Mat& F_mtx) // Compute F-mtx from known P-mtx
    {
        //---Derive F-mtx by known P-mtx---
            // E = B * R_1to2 = R_2to1 * B
            // F = inv(K2)' * E * inv(K)
            // F = [epipole2]_skewSym * P2 * inv(P1) = [P2*PC1]_skewSym * P2 * inv(P1)

        cv::Mat P_ImgB_PsudoInv;
        cv::invert(P_mtx, P_ImgB_PsudoInv, cv::DECOMP_SVD);

        cv::Mat PC_ImgB = cv::Mat(4, 1, CV_32F);
        PC_ImgB.at<float>(0, 0) = P_mtx.at<float>(0, 3);
        PC_ImgB.at<float>(1, 0) = P_mtx.at<float>(1, 3);
        PC_ImgB.at<float>(2, 0) = P_mtx.at<float>(2, 3);
        PC_ImgB.at<float>(3, 0) = 1;

        cv::Mat EpiPole_ImgM = P_ImgM * PC_ImgB; // Epipole of Image_Match
        cv::Mat Epipole_ImgM_SkewSymMtx = cv::Mat::zeros(cv::Size(3, 3), CV_32F);
        Epipole_ImgM_SkewSymMtx.at<float>(0, 1) = -EpiPole_ImgM.at<float>(2, 0);
        Epipole_ImgM_SkewSymMtx.at<float>(0, 2) =  EpiPole_ImgM.at<float>(1, 0);
        Epipole_ImgM_SkewSymMtx.at<float>(1, 0) =  EpiPole_ImgM.at<float>(2, 0);
        Epipole_ImgM_SkewSymMtx.at<float>(1, 2) = -EpiPole_ImgM.at<float>(0, 0);
        Epipole_ImgM_SkewSymMtx.at<float>(2, 0) = -EpiPole_ImgM.at<float>(1, 0);
        Epipole_ImgM_SkewSymMtx.at<float>(2, 1) =  EpiPole_ImgM.at<float>(0, 0);

        F_mtx = Epipole_ImgM_SkewSymMtx * P_ImgM  * P_ImgB_PsudoInv;
    }

    //---Set Functions---
    void FutoGmtCV::set_Cam(cv::Mat& K_Input)
    {
        K_mtx = K_Input;
    }

    void FutoGmtCV::set_Rot(cv::Mat& R_Input)
    {
        Rot_mtx = R_Input;
    }

    void FutoGmtCV::set_Trans(cv::Mat& T_Input)
    {
        Trans_vec = T_Input;
    }


    void FutoGmtCV::set_P_mtx(cv::Mat P_Input)
    {
        P_mtx = P_Input; // P_mtx = K_mtx * [Rot_mtx | Trans_vec]
    }


    //---Get Function---
    cv::Mat FutoGmtCV::get_Img()
    {
        return Img;
    }

    cv::Mat FutoGmtCV::get_P_mtx()
    {
        return P_mtx;
    }

    //---Is Function---
    bool FutoGmtCV::is_InsideImg(cv::Point2f ImgPt, cv::Size ImgSize)
    {
        return ( (ImgPt.x >= 0) && (ImgPt.y >= 0) && (ImgPt.x < ImgSize.width) && (ImgPt.y < ImgSize.height) );
    }

    //---Export Function---
    void FutoGmtCV::show_Img()
    {
        cv::imshow("Image", Img);
    }

} // Stereo
