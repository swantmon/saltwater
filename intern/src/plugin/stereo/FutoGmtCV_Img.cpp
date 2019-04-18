#include "plugin\stereo\stereo_precompiled.h"
#include "plugin\stereo\FutoGmtCV_Img.h"


namespace FutoGmtCV
{
    //---Constructors & Destructor---
    FutoImg::FutoImg()
    {
    }

    FutoImg::FutoImg(cv::Mat& Img_Input)
        : m_Img(Img_Input)
    {
    }

    FutoImg::FutoImg(cv::Mat& Img_Input, cv::Mat P)
        : m_Img(Img_Input),
          m_P_mtx(P)
    {
        // Derive K, R, T from P

    }
    FutoImg::FutoImg(cv::Mat& Img_Input, cv::Mat K, cv::Mat R, cv::Mat PC)
        : m_Img(Img_Input),
          m_Cam_mtx(K),
          m_Rot_mtx(R), 
          m_PC_vec(PC)
    {
        cv::Mat Transform_mtx(3, 4, CV_32F);
        m_Rot_mtx.colRange(0, 3).copyTo(Transform_mtx.colRange(0, 3));
        cv::Mat Trans_vec = -m_Rot_mtx * PC;
        Trans_vec.col(0).copyTo(Transform_mtx.col(3));

        m_P_mtx = m_Cam_mtx * Transform_mtx;
    }

    FutoImg::~FutoImg()
    {
    }

    //---Set Functions---
    void FutoImg::set_Cam(cv::Mat& K_Input)
    {
        m_Cam_mtx = K_Input;
    }

    void FutoImg::set_Rot(cv::Mat& R_Input)
    {
        m_Rot_mtx = R_Input;
    }

    void FutoImg::set_PC(cv::Mat& PC_Input)
    {
        m_PC_vec = PC_Input;
    }


    void FutoImg::set_PPM(cv::Mat P_Input)
    {
        m_P_mtx = P_Input; // P_mtx = K_mtx * [Rot_mtx | Trans_vec] = K_mtx * [Rot_mtx | -Rot_mtx * PC_vec]
    }


    //---Get Function---
    cv::Mat FutoImg::get_Img() const
    {
        return m_Img;
    }

    cv::Mat FutoImg::get_Cam() const
    {
        return m_Cam_mtx;
    }

    cv::Mat FutoImg::get_Rot() const
    {
        return m_Rot_mtx;
    }

    cv::Mat FutoImg::get_PC() const
    {
        return m_PC_vec;
    }

    cv::Mat FutoImg::get_PPM() const
    {
        return m_P_mtx;
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
