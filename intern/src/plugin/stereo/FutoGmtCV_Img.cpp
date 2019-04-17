#include "plugin\stereo\stereo_precompiled.h"
#include "plugin\stereo\FutoGmtCV_Img.h"


namespace FutoGmtCV
{
    //---Constructors & Destructor---
    FutoImg::FutoImg()
    {
    }

    FutoImg::FutoImg(cv::Mat& Img_Input)
        : Img_RGB(Img_Input)
    {
    }

    FutoImg::FutoImg(cv::Mat& Img_Input, cv::Mat P)
        : Img_RGB(Img_Input),
          P_mtx(P)
    {
        // Derive K, R, T from P

    }
    FutoImg::FutoImg(cv::Mat& Img_Input, cv::Mat K, cv::Mat R, cv::Mat PC)
        : Img_RGB(Img_Input),
          K_mtx(K),
          Rot_mtx(R), 
          PC_vec(PC)
    {
        cv::Mat Transform_mtx(3, 4, CV_32F);
        Rot_mtx.colRange(0, 3).copyTo(Transform_mtx.colRange(0, 3));
        cv::Mat Trans_vec = -Rot_mtx * PC;
        Trans_vec.col(0).copyTo(Transform_mtx.col(3));

        P_mtx = K_mtx * Transform_mtx;
    }

    FutoImg::~FutoImg()
    {
    }

    //---Set Functions---
    void FutoImg::set_Cam(cv::Mat& K_Input)
    {
        K_mtx = K_Input;
    }

    void FutoImg::set_Rot(cv::Mat& R_Input)
    {
        Rot_mtx = R_Input;
    }

    void FutoImg::set_PC(cv::Mat& PC_Input)
    {
        PC_vec = PC_Input;
    }


    void FutoImg::set_PPM(cv::Mat P_Input)
    {
        P_mtx = P_Input; // P_mtx = K_mtx * [Rot_mtx | Trans_vec] = K_mtx * [Rot_mtx | -Rot_mtx * PC_vec]
    }


    //---Get Function---
    cv::Mat FutoImg::get_Img() const
    {
        return Img_RGB;
    }

    cv::Mat FutoImg::get_Cam() const
    {
        return K_mtx;
    }

    cv::Mat FutoImg::get_Rot() const
    {
        return Rot_mtx;
    }

    cv::Mat FutoImg::get_PC() const
    {
        return PC_vec;
    }

    cv::Mat FutoImg::get_PPM() const
    {
        return P_mtx;
    }


    //===== OLD =====

    //---Photogrammetric Computer Vision---
    void FutoImg::imp_cvSGBM(cv::Mat& DispImg, const cv::Mat& RectImg_Base, const cv::Mat& RectImg_Match)
    {
        operPtr_cvSGBM = cv::StereoSGBM::create();
        int WinSize_SAD = 9;
        operPtr_cvSGBM->setBlockSize(WinSize_SAD);
        operPtr_cvSGBM->setP1(8 * WinSize_SAD * WinSize_SAD);
        operPtr_cvSGBM->setP2(32 * WinSize_SAD * WinSize_SAD);
        operPtr_cvSGBM->setPreFilterCap(8);
        operPtr_cvSGBM->setDisp12MaxDiff(1);
        operPtr_cvSGBM->setUniquenessRatio(10);
        operPtr_cvSGBM->setSpeckleWindowSize(100);
        operPtr_cvSGBM->setSpeckleRange(1);
        //operPtr_cvSGBM->setMode(cv::StereoSGBM::MODE_SGBM);

        operPtr_cvSGBM->compute(RectImg_Base, RectImg_Match, DispImg);
    }

} // FutoImg
