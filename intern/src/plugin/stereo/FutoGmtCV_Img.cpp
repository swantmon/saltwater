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

    cv::Mat FutoImg::get_P_mtx() const
    {
        return P_mtx;
    }


    //===== OLD =====

    //---Photogrammetric Computer Vision---
    void FutoImg::imp_PolarRect(cv::Mat& RectImg_Base, cv::Mat& RectImg_Match, const cv::Mat& Img_Match, const cv::Mat F_mtx) // Epipolarization based on Polar Rectification
    {
        operObj_PolarRect = Rect_Polar(Img_RGB, Img_Match);

        operObj_PolarRect.compute(F_mtx);

        operObj_PolarRect.genrt_RectImg();

        operObj_PolarRect.get_RectImg(RectImg_Base, RectImg_Match);
    }

    void FutoImg::imp_PlanarRect(FutoImg &RectImg_Base, FutoImg &RectImg_Match, cv::Mat &Orig2Rect_B_x, cv::Mat &Orig2Rect_B_y, cv::Mat &Orig2Rect_M_x, cv::Mat &Orig2Rect_M_y, const FutoImg &OrigImg_Match)
    {
        //---Calculate the Orientations of Rectified Images---
        operObj_PlanarRect.cal_K_Rect(K_mtx, OrigImg_Match.get_Cam());
        operObj_PlanarRect.cal_R_Rect(Rot_mtx, PC_vec, OrigImg_Match.get_PC());
        operObj_PlanarRect.cal_P_Rect(PC_vec, OrigImg_Match.get_PC());
        operObj_PlanarRect.cal_H(P_mtx, OrigImg_Match.get_P_mtx());

        //---Center the Rectified Images---
        operObj_PlanarRect.imp_CenterRectImg(Img_RGB.size(), OrigImg_Match.get_Img().size());
        operObj_PlanarRect.cal_H(P_mtx, OrigImg_Match.get_P_mtx());

        //---Generate Rectified Images---
        operObj_PlanarRect.determ_RectImgSize(Img_RGB.size(), OrigImg_Match.get_Img().size());
        operObj_PlanarRect.genrt_RectImg(Img_RGB, OrigImg_Match.get_Img());

        //---Derive Rectified Images---
        cv::Mat RectImg_B, CamB_Rect, RectImg_M, CamM_Rect, Rot_Rect;
        operObj_PlanarRect.get_RectImg(RectImg_B, RectImg_M);
        operObj_PlanarRect.get_K_Rect(CamB_Rect, CamM_Rect);
        operObj_PlanarRect.get_R_Rect(Rot_Rect);
        RectImg_Base = FutoImg(RectImg_B, CamB_Rect, Rot_Rect, PC_vec);
        RectImg_Match = FutoImg(RectImg_M, CamM_Rect, Rot_Rect, OrigImg_Match.get_PC());

        operObj_PlanarRect.get_Transform_Orig2Rect(Orig2Rect_B_x, Orig2Rect_B_y, Orig2Rect_M_x, Orig2Rect_M_y);
    }

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

    //---Orientation & Transformation---
    void FutoImg::cal_F_mtx(const cv::Mat& P_ImgM, cv::Mat& F_mtx) // Compute F-mtx from known P-mtx
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

    void FutoImg::cal_F_mtx(const cv::Mat& K_ImgM, const cv::Mat& R_ImgM, const cv::Mat& PC_ImgM, cv::Mat& F_mtx)
    {
        cv::Mat b = PC_ImgM - PC_vec;
        b = -Rot_mtx * b;
        cv::Mat B_SkewSym = cv::Mat::zeros(cv::Size(3, 3), CV_32F);
        B_SkewSym.at<float>(0, 1) = -b.at<float>(2, 0);
        B_SkewSym.at<float>(0, 2) = b.at<float>(1, 0);
        B_SkewSym.at<float>(1, 0) = b.at<float>(2, 0);
        B_SkewSym.at<float>(1, 2) = -b.at<float>(0, 0);
        B_SkewSym.at<float>(2, 0) = -b.at<float>(1, 0);
        B_SkewSym.at<float>(2, 1) = b.at<float>(0, 0);

        cv::Mat R_B2M = R_ImgM * Rot_mtx.t();

        cv::Mat E_mtx = B_SkewSym * R_B2M;

        F_mtx = K_ImgM.inv().t() * E_mtx * K_mtx.inv();
    }

} // FutoImg
