#include "plugin\stereo\stereo_precompiled.h"
#include "plugin\stereo\stereo_futogmtcv.h"


namespace Stereo
{
    //---Constructors & Destructor---
    FutoGmtCV::FutoGmtCV()
    {
    }

    FutoGmtCV::FutoGmtCV(cv::Mat& Img_Input)
        : Img(Img_Input)
    {
    }

    FutoGmtCV::FutoGmtCV(cv::Mat& Img_Input, cv::Mat P)
        : Img(Img_Input),
          P_mtx(P)
    {
        // Derive K, R, T from P

    }
    FutoGmtCV::FutoGmtCV(cv::Mat& Img_Input, cv::Mat K, cv::Mat R, cv::Mat T)
        : Img(Img_Input),
          K_mtx(K),
          Rot_mtx(R), 
          Trans_vec(T)
    {
        cv::Mat Transform_mtx(3, 4, CV_32F);
        Rot_mtx.colRange(0, 3).copyTo(Transform_mtx.colRange(0, 3));
        Trans_vec.col(0).copyTo(Transform_mtx.col(3));

        P_mtx = K_mtx * Transform_mtx;
    }

    FutoGmtCV::~FutoGmtCV()
    {
    }

    //---Photogrammetric Computer Vision---
    void FutoGmtCV::imp_PolarRect(cv::Mat& RectImg_Base, cv::Mat& RectImg_Match, const cv::Mat& Img_Match, const cv::Mat F_mtx) // Epipolarization based on Polar Rectification
    {
        operObj_PolarRect = Rect_Polar(Img, Img_Match);

        operObj_PolarRect.compute(F_mtx);

        operObj_PolarRect.genrt_RectImg();

        operObj_PolarRect.get_RectImg(RectImg_Base, RectImg_Match);
    }

    void FutoGmtCV::imp_PlanarRect(cv::Mat& RectImg_Base, cv::Mat& RectImg_Match, cv::Mat& Orig2Rect_B_x, cv::Mat& Orig2Rect_B_y, cv::Mat& Orig2Rect_M_x, cv::Mat& Orig2Rect_M_y, const FutoGmtCV& OrigImg_Match)
    {
        operObj_PlanarRect.cal_K_Rect(K_mtx, OrigImg_Match.get_Cam());
        operObj_PlanarRect.cal_R_Rect(Rot_mtx, Trans_vec, OrigImg_Match.get_Trans());
        operObj_PlanarRect.cal_P_Rect(Trans_vec, OrigImg_Match.get_Trans());
        operObj_PlanarRect.cal_H(P_mtx, OrigImg_Match.get_P_mtx());

        operObj_PlanarRect.determ_RectImgSize(Img.size(), OrigImg_Match.get_Img().size());
        operObj_PlanarRect.genrt_RectImg(Img, OrigImg_Match.get_Img());
        operObj_PlanarRect.get_RectImg(RectImg_Base, RectImg_Match);
        operObj_PlanarRect.get_Transform_Orig2Rect(Orig2Rect_B_x, Orig2Rect_B_y, Orig2Rect_M_x, Orig2Rect_M_y);
    }

    void FutoGmtCV::imp_cvSGBM(cv::Mat& DispImg, const cv::Mat& RectImg_Base, const cv::Mat& RectImg_Match)
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
    cv::Mat FutoGmtCV::get_Img() const
    {
        return Img;
    }

    cv::Mat FutoGmtCV::get_Cam() const
    {
        return K_mtx;
    }

    cv::Mat FutoGmtCV::get_Trans() const
    {
        return Trans_vec;
    }

    cv::Mat FutoGmtCV::get_P_mtx() const
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
