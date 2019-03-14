#include "plugin\stereo\stereo_precompiled.h"
#include "plugin\stereo\stereo_Rect_Planar.h"

namespace Stereo
{
    //---Constructors & Destructor---
    Rect_Planar::Rect_Planar()
    {
    }


    Rect_Planar::~Rect_Planar()
    {
    }

    //---Generation of Rectified Img---
    void Rect_Planar::genrt_RectifiedImg(const cv::Mat& Img_Orig_B, const cv::Mat& Img_Orig_M)
    {
        //---Create Look-Up Table for Transformation from Original to Rectified Images---
        mapB_x_Orig2Rect = cv::Mat(ImgSize_Rect_B, CV_32FC1);
        mapB_y_Orig2Rect = cv::Mat(ImgSize_Rect_B, CV_32FC1);
        for (int x = 0; x < ImgSize_Rect_B.width; x++)
        {
            for (int y = 0; y < ImgSize_Rect_B.height; y++)
            {
                cv::Mat Pix_Rect(3, 1, CV_32F);
                Pix_Rect .col(0) = (x, y, 1);
                cv::Mat Pix_Rect2Orig = H_B.inv() * Pix_Rect;
                Pix_Rect2Orig = Pix_Rect2Orig / Pix_Rect2Orig.at<float>(2, 0);
                mapB_x_Orig2Rect.at<float>(x, y) = Pix_Rect2Orig.at<float>(0, 0);
                mapB_y_Orig2Rect.at<float>(x, y) = Pix_Rect2Orig.at<float>(1, 0);
            }
        }

        mapM_x_Orig2Rect = cv::Mat(ImgSize_Rect_M, CV_32FC1);
        mapM_y_Orig2Rect = cv::Mat(ImgSize_Rect_M, CV_32FC1);
        for (int x = 0; x < ImgSize_Rect_M.width; x++)
        {
            for (int y = 0; y < ImgSize_Rect_M.height; y++)
            {
                cv::Mat Pix_Rect(3, 1, CV_32F);
                Pix_Rect.col(0) = (x, y, 1);
                cv::Mat Pix_Rect2Orig = H_M.inv() * Pix_Rect;
                Pix_Rect2Orig = Pix_Rect2Orig / Pix_Rect2Orig.at<float>(2, 0);
                mapM_x_Orig2Rect.at<float>(x, y) = Pix_Rect2Orig.at<float>(0, 0);
                mapM_y_Orig2Rect.at<float>(x, y) = Pix_Rect2Orig.at<float>(1, 0);
            }
        }

        //---Generate Rectified Images---

        Img_Rect_B = cv::Mat(ImgSize_Rect_B, CV_8UC4);
        cv::remap(Img_Orig_B, Img_Rect_B, mapB_x_Orig2Rect, mapB_y_Orig2Rect, cv::INTER_LINEAR, cv::BORDER_TRANSPARENT);

        Img_Rect_M = cv::Mat(ImgSize_Rect_M, CV_8UC4);
        cv::remap(Img_Orig_M, Img_Rect_M, mapM_x_Orig2Rect, mapM_y_Orig2Rect, cv::INTER_LINEAR, cv::BORDER_TRANSPARENT);

        //---Create Look-Up Table for Transformation from Rectified & Original Images---
        mapB_x_Rect2Orig = cv::Mat(Img_Orig_B.size(), CV_32FC1);
        mapB_y_Rect2Orig = cv::Mat(Img_Orig_B.size(), CV_32FC1);
        for (int x = 0; x < Img_Orig_B.size().width; x++)
        {
            for (int y = 0; y < Img_Orig_B.size().height; y++)
            {
                cv::Mat Pix_Orig(3, 1, CV_32F);
                Pix_Orig.col(0) = (x, y, 1);
                cv::Mat Pix_Orig2Rect = H_B * Pix_Orig;
                Pix_Orig2Rect = Pix_Orig2Rect / Pix_Orig2Rect.at<float>(2, 0);
                mapB_x_Rect2Orig.at<float>(x, y) = Pix_Orig2Rect.at<float>(0, 0);
                mapB_y_Rect2Orig.at<float>(x, y) = Pix_Orig2Rect.at<float>(1, 0);
            }
        }

        mapM_x_Rect2Orig = cv::Mat(Img_Orig_M.size(), CV_32FC1);
        mapM_y_Rect2Orig = cv::Mat(Img_Orig_M.size(), CV_32FC1);
        for (int x = 0; x < Img_Orig_M.size().width; x++)
        {
            for (int y = 0; y < Img_Orig_M.size().height; y++)
            {
                cv::Mat Pix_Orig(3, 1, CV_32F);
                Pix_Orig.col(0) = (x, y, 1);
                cv::Mat Pix_Orig2Rect = H_M * Pix_Orig;
                Pix_Orig2Rect = Pix_Orig2Rect / Pix_Orig2Rect.at<float>(2, 0);
                mapM_x_Rect2Orig.at<float>(x, y) = Pix_Orig2Rect.at<float>(0, 0);
                mapM_y_Rect2Orig.at<float>(x, y) = Pix_Orig2Rect.at<float>(1, 0);
            }
        }
    }

    void Rect_Planar::determ_RectiedImgSize(const cv::Size& ImgSize_OrigB, const cv::Size& ImgSize_OrigM)
    {
        cv::Mat ImgCorner_OrigB_UL(3, 1, CV_32F), ImgCorner_OrigB_UR(3, 1, CV_32F), ImgCorner_OrigB_DL(3, 1, CV_32F), ImgCorner_OrigB_DR(3, 1, CV_32F);
        ImgCorner_OrigB_UL.col(0) = (0, 0, 1);
        ImgCorner_OrigB_UR.col(0) = (ImgSize_OrigB.width, 0, 1);
        ImgCorner_OrigB_DL.col(0) = (0, ImgSize_OrigB.height, 1);
        ImgCorner_OrigB_DR.col(0) = (ImgSize_OrigB.width, ImgSize_OrigB.height, 1);

        cv::Mat ImgCorner_OrigB2RectB_UL = H_B * ImgCorner_OrigB_UL;
        ImgCorner_OrigB2RectB_UL = ImgCorner_OrigB2RectB_UL / ImgCorner_OrigB2RectB_UL.at<float>(2, 0);
        cv::Mat ImgCorner_OrigB2RectB_UR = H_B * ImgCorner_OrigB_UR;
        ImgCorner_OrigB2RectB_UR = ImgCorner_OrigB2RectB_UR / ImgCorner_OrigB2RectB_UR.at<float>(2, 0);
        cv::Mat ImgCorner_OrigB2RectB_DL = H_B * ImgCorner_OrigB_DL;
        ImgCorner_OrigB2RectB_DL = ImgCorner_OrigB2RectB_DL / ImgCorner_OrigB2RectB_DL.at<float>(2, 0);
        cv::Mat ImgCorner_OrigB2RectB_DR = H_B * ImgCorner_OrigB_DR;
        ImgCorner_OrigB2RectB_DR = ImgCorner_OrigB2RectB_DR / ImgCorner_OrigB2RectB_DR.at<float>(2, 0);

        float ImgBound_RectB_x_min, ImgBound_RectB_x_max, ImgBound_RectB_y_min, ImgBound_RectB_y_max;
        ImgBound_RectB_x_min = ImgCorner_OrigB2RectB_UL.at<float>(0, 0) <= ImgCorner_OrigB2RectB_DL.at<float>(0, 0) ? ImgCorner_OrigB2RectB_UL.at<float>(0, 0) : ImgCorner_OrigB2RectB_DL.at<float>(0, 0);
        ImgBound_RectB_x_max = ImgCorner_OrigB2RectB_UR.at<float>(0, 0) >= ImgCorner_OrigB2RectB_DR.at<float>(0, 0) ? ImgCorner_OrigB2RectB_UR.at<float>(0, 0) : ImgCorner_OrigB2RectB_DR.at<float>(0, 0);
        ImgBound_RectB_y_min = ImgCorner_OrigB2RectB_UL.at<float>(1, 0) <= ImgCorner_OrigB2RectB_UR.at<float>(1, 0) ? ImgCorner_OrigB2RectB_UL.at<float>(1, 0) : ImgCorner_OrigB2RectB_UR.at<float>(1, 0);
        ImgBound_RectB_y_max = ImgCorner_OrigB2RectB_DL.at<float>(1, 0) >= ImgCorner_OrigB2RectB_DR.at<float>(1, 0) ? ImgCorner_OrigB2RectB_DL.at<float>(1, 0) : ImgCorner_OrigB2RectB_DR.at<float>(1, 0);

        ImgSize_Rect_B = cv::Size(ImgBound_RectB_x_max - ImgBound_RectB_x_min, ImgBound_RectB_y_max - ImgBound_RectB_y_min);

        cv::Mat ImgCorner_OrigM_UL(3, 1, CV_32F), ImgCorner_OrigM_UR(3, 1, CV_32F), ImgCorner_OrigM_DL(3, 1, CV_32F), ImgCorner_OrigM_DR(3, 1, CV_32F);
        ImgCorner_OrigM_UL.col(0) = (0, 0, 1);
        ImgCorner_OrigM_UR.col(0) = (ImgSize_OrigM.width, 0, 1);
        ImgCorner_OrigM_DL.col(0) = (0, ImgSize_OrigM.height, 1);
        ImgCorner_OrigM_DR.col(0) = (ImgSize_OrigM.width, ImgSize_OrigM.height, 1);

        cv::Mat ImgCorner_OrigM2RectM_UL = H_M * ImgCorner_OrigM_UL;
        ImgCorner_OrigM2RectM_UL = ImgCorner_OrigM2RectM_UL / ImgCorner_OrigM2RectM_UL.at<float>(2, 0);
        cv::Mat ImgCorner_OrigM2RectM_UR = H_M * ImgCorner_OrigM_UR;
        ImgCorner_OrigM2RectM_UR = ImgCorner_OrigM2RectM_UR / ImgCorner_OrigM2RectM_UR.at<float>(2, 0);
        cv::Mat ImgCorner_OrigM2RectM_DL = H_M * ImgCorner_OrigM_DL;
        ImgCorner_OrigM2RectM_DL = ImgCorner_OrigM2RectM_DL / ImgCorner_OrigM2RectM_DL.at<float>(2, 0);
        cv::Mat ImgCorner_OrigM2RectM_DR = H_M * ImgCorner_OrigM_DR;
        ImgCorner_OrigM2RectM_DR = ImgCorner_OrigM2RectM_DR / ImgCorner_OrigM2RectM_DR.at<float>(2, 0);

        float ImgBound_RectM_x_min, ImgBound_RectM_x_max, ImgBound_RectM_y_min, ImgBound_RectM_y_max;
        ImgBound_RectM_x_min = ImgCorner_OrigM2RectM_UL.at<float>(0, 0) <= ImgCorner_OrigM2RectM_DL.at<float>(0, 0) ? ImgCorner_OrigM2RectM_UL.at<float>(0, 0) : ImgCorner_OrigM2RectM_DL.at<float>(0, 0);
        ImgBound_RectM_x_max = ImgCorner_OrigM2RectM_UR.at<float>(0, 0) >= ImgCorner_OrigM2RectM_DR.at<float>(0, 0) ? ImgCorner_OrigM2RectM_UR.at<float>(0, 0) : ImgCorner_OrigM2RectM_DR.at<float>(0, 0);
        ImgBound_RectM_y_min = ImgCorner_OrigM2RectM_UL.at<float>(1, 0) <= ImgCorner_OrigM2RectM_UR.at<float>(1, 0) ? ImgCorner_OrigM2RectM_UL.at<float>(1, 0) : ImgCorner_OrigM2RectM_UR.at<float>(1, 0);
        ImgBound_RectM_y_max = ImgCorner_OrigM2RectM_DL.at<float>(1, 0) >= ImgCorner_OrigM2RectM_DR.at<float>(1, 0) ? ImgCorner_OrigM2RectM_DL.at<float>(1, 0) : ImgCorner_OrigM2RectM_DR.at<float>(1, 0);

        ImgSize_Rect_M = cv::Size(ImgBound_RectM_x_max - ImgBound_RectM_x_min, ImgBound_RectM_y_max - ImgBound_RectM_y_min);
    }

    //---Compute Orientations---
    void Rect_Planar::cal_K_Rect(const cv::Mat& K_Orig_B, const cv::Mat& K_Orig_M)
    {
        K_Rect = 0.5 * (K_Orig_B + K_Orig_M);
    }

    void Rect_Planar::cal_R_Rect(const cv::Mat& R_Orig_B, const cv::Mat& t_Orig_B, const cv::Mat& t_Orig_M)
    {
        R_Rect.row(0) = (t_Orig_B - t_Orig_M) / cv::norm((t_Orig_B - t_Orig_M), cv::NORM_L1);

        R_Rect.row(1) = R_Orig_B.row(2).cross(R_Rect.row(0));

        R_Rect.row(2) = R_Rect.row(0).cross(R_Rect.row(1));
    }

    void Rect_Planar::cal_P_Rect(const cv::Mat& t_Orig_B, const cv::Mat& t_Orig_M)
    {
        cv::Mat Trans_Rect_B(3, 4, CV_32F);
        Trans_Rect_B.colRange(0, 2) = R_Rect.colRange(0, 2);
        Trans_Rect_B.col(3) = t_Orig_B.col(0);
        cv::Mat Trans_Rect_M(3, 4, CV_32F);
        Trans_Rect_M.colRange(0, 2) = R_Rect.colRange(0, 2);
        Trans_Rect_M.col(3) = t_Orig_M.col(0);

        P_Rect_B = K_Rect * Trans_Rect_B;
        P_Rect_M = K_Rect * Trans_Rect_M;
    }

    void Rect_Planar::cal_H(const cv::Mat& P_Orig_B, cv::Mat& P_Orig_M)
    {
        H_B = P_Rect_B * P_Orig_B.inv(cv::DECOMP_SVD);
        H_M = P_Rect_M * P_Orig_M.inv(cv::DECOMP_SVD);
    }

    //---Get Function---
    void Rect_Planar::get_RectImg(cv::Mat& Output_RectImgB, cv::Mat& Output_RectImgM)
    {
        Output_RectImgB = Img_Rect_B;
        Output_RectImgM = Img_Rect_M;
    }

} // Stereo
