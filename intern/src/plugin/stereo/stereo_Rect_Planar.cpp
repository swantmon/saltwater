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
        cv::Mat ImgCnrUL_OrigB = cv::Mat::ones(3, 1, CV_32F);
        ImgCnrUL_OrigB.at<float>(0, 0) = 0;
        ImgCnrUL_OrigB.at<float>(1, 0) = 0;
        cv::Mat ImgCnrUR_OrigB = cv::Mat::ones(3, 1, CV_32F);
        ImgCnrUR_OrigB.at<float>(0, 0) = ImgSize_OrigB.width;
        ImgCnrUR_OrigB.at<float>(1, 0) = 0;
        cv::Mat ImgCnrDL_OrigB = cv::Mat::ones(3, 1, CV_32F);
        ImgCnrDL_OrigB.at<float>(0, 0) = 0;
        ImgCnrDL_OrigB.at<float>(1, 0) = ImgSize_OrigB.height;
        cv::Mat ImgCnrDR_OrigB = cv::Mat::ones(3, 1, CV_32F);
        ImgCnrDR_OrigB.at<float>(0, 0) = ImgSize_OrigB.width;
        ImgCnrDR_OrigB.at<float>(1, 0) = ImgSize_OrigB.height;

        cv::Mat ImgCnrUL_Orig2Rect_B = H_B * ImgCnrUL_OrigB;
        ImgCnrUL_Orig2Rect_B = ImgCnrUL_Orig2Rect_B / ImgCnrUL_Orig2Rect_B.at<float>(2, 0);
        float OrigB2RectB_UL00 = ImgCnrUL_Orig2Rect_B.at<float>(0, 0);
        float OrigB2RectB_UL10 = ImgCnrUL_Orig2Rect_B.at<float>(1, 0);
        float OrigB2RectB_UL20 = ImgCnrUL_Orig2Rect_B.at<float>(2, 0);
        cv::Mat ImgCnrUR_Orig2Rect_B = H_B * ImgCnrUR_OrigB;
        ImgCnrUR_Orig2Rect_B = ImgCnrUR_Orig2Rect_B / ImgCnrUR_Orig2Rect_B.at<float>(2, 0);
        float OrigB2RectB_UR00 = ImgCnrUR_Orig2Rect_B.at<float>(0, 0);
        float OrigB2RectB_UR10 = ImgCnrUR_Orig2Rect_B.at<float>(1, 0);
        float OrigB2RectB_UR20 = ImgCnrUR_Orig2Rect_B.at<float>(2, 0);
        cv::Mat ImgCnrDL_Orig2Rect_B = H_B * ImgCnrDL_OrigB;
        ImgCnrDL_Orig2Rect_B = ImgCnrDL_Orig2Rect_B / ImgCnrDL_Orig2Rect_B.at<float>(2, 0);
        float OrigB2RectB_DL00 = ImgCnrDL_Orig2Rect_B.at<float>(0, 0);
        float OrigB2RectB_DL10 = ImgCnrDL_Orig2Rect_B.at<float>(1, 0);
        float OrigB2RectB_DL20 = ImgCnrDL_Orig2Rect_B.at<float>(2, 0);
        cv::Mat ImgCnrDR_Orig2Rect_B = H_B * ImgCnrDR_OrigB;
        ImgCnrDR_Orig2Rect_B = ImgCnrDR_Orig2Rect_B / ImgCnrDR_Orig2Rect_B.at<float>(2, 0);
        float OrigB2RectB_DR00 = ImgCnrDR_Orig2Rect_B.at<float>(0, 0);
        float OrigB2RectB_DR10 = ImgCnrDR_Orig2Rect_B.at<float>(1, 0);
        float OrigB2RectB_DR20 = ImgCnrDR_Orig2Rect_B.at<float>(2, 0);

        float ImgBound_RectB_x_min = std::min(ImgCnrUL_Orig2Rect_B.at<float>(0, 0), ImgCnrDL_Orig2Rect_B.at<float>(0, 0));
        float ImgBound_RectB_y_min = std::min(ImgCnrUL_Orig2Rect_B.at<float>(1, 0), ImgCnrUR_Orig2Rect_B.at<float>(1, 0));
        float ImgBound_RectB_x_max = std::max(ImgCnrUR_Orig2Rect_B.at<float>(0, 0), ImgCnrDR_Orig2Rect_B.at<float>(0, 0));
        float ImgBound_RectB_y_max = std::max(ImgCnrDL_Orig2Rect_B.at<float>(1, 0), ImgCnrDR_Orig2Rect_B.at<float>(1, 0));

        //ImgSize_Rect_B = cv::Size(ImgBound_RectB_x_max, ImgBound_RectB_y_max);

        cv::Mat ImgCnrUL_OrigM = cv::Mat::ones(3, 1, CV_32F);
        ImgCnrUL_OrigM.at<float>(0, 0) = 0;
        ImgCnrUL_OrigM.at<float>(1, 0) = 0;
        cv::Mat ImgCnrUR_OrigM = cv::Mat::ones(3, 1, CV_32F);
        ImgCnrUR_OrigM.at<float>(0, 0) = ImgSize_OrigM.width;
        ImgCnrUR_OrigM.at<float>(1, 0) = 0;
        cv::Mat ImgCnrDL_OrigM = cv::Mat::ones(3, 1, CV_32F);
        ImgCnrDL_OrigM.at<float>(0, 0) = 0;
        ImgCnrDL_OrigM.at<float>(1, 0) = ImgSize_OrigM.height;
        cv::Mat ImgCnrDR_OrigM = cv::Mat::ones(3, 1, CV_32F);
        ImgCnrDR_OrigM.at<float>(0, 0) = ImgSize_OrigM.width;
        ImgCnrDR_OrigM.at<float>(1, 0) = ImgSize_OrigM.height;

        cv::Mat ImgCnrUL_Orig2Rect_M = H_M * ImgCnrUL_OrigM;
        ImgCnrUL_Orig2Rect_M = ImgCnrUL_Orig2Rect_M / ImgCnrUL_Orig2Rect_M.at<float>(2, 0);
        
        cv::Mat ImgCnrUR_Orig2Rect_M = H_M * ImgCnrUR_OrigM;
        ImgCnrUR_Orig2Rect_M = ImgCnrUR_Orig2Rect_M / ImgCnrUR_Orig2Rect_M.at<float>(2, 0);
        
        cv::Mat ImgCnrDL_Orig2Rect_M = H_M * ImgCnrDL_OrigM;
        ImgCnrDL_Orig2Rect_M = ImgCnrDL_Orig2Rect_M / ImgCnrDL_Orig2Rect_M.at<float>(2, 0);
        
        cv::Mat ImgCnrDR_Orig2Rect_M = H_M * ImgCnrDR_OrigM;
        ImgCnrDR_Orig2Rect_M = ImgCnrDR_Orig2Rect_M / ImgCnrDR_Orig2Rect_M.at<float>(2, 0);

        float ImgBound_RectM_x_min = std::min(ImgCnrUL_Orig2Rect_M.at<float>(0, 0), ImgCnrDL_Orig2Rect_M.at<float>(0, 0));
        float ImgBound_RectM_y_min = std::min(ImgCnrUL_Orig2Rect_M.at<float>(1, 0), ImgCnrUR_Orig2Rect_M.at<float>(1, 0));
        float ImgBound_RectM_x_max = std::max(ImgCnrUR_Orig2Rect_M.at<float>(0, 0), ImgCnrDR_Orig2Rect_M.at<float>(0, 0));
        float ImgBound_RectM_y_max = std::max(ImgCnrDL_Orig2Rect_M.at<float>(1, 0), ImgCnrDR_Orig2Rect_M.at<float>(1, 0));

        //ImgSize_Rect_M = cv::Size(ImgBound_RectM_x_max - ImgBound_RectM_x_min, ImgBound_RectM_y_max - ImgBound_RectM_y_min);
    }

    //---Compute Orientations---
    void Rect_Planar::cal_K_Rect(const cv::Mat& K_Orig_B, const cv::Mat& K_Orig_M)
    {
        K_Rect = 0.5 * (K_Orig_B + K_Orig_M);
    }

    void Rect_Planar::cal_R_Rect(const cv::Mat& R_Orig_B, const cv::Mat& t_Orig_B, const cv::Mat& t_Orig_M)
    {
        R_Rect = cv::Mat(3, 3, CV_32F);

        cv::Mat R_Rect_row0 = t_Orig_B - t_Orig_M;
        R_Rect_row0 = R_Rect_row0 / cv::norm(R_Rect_row0, cv::NORM_L2);
        cv::transpose(R_Rect_row0, R_Rect_row0);

        cv::Mat R_Rect_row1 = R_Orig_B.row(2).cross(R_Rect_row0);
        R_Rect_row1 = R_Rect_row1 / cv::norm(R_Rect_row1, cv::NORM_L2);

        cv::Mat R_Rect_row2 = R_Rect_row0.cross(R_Rect_row1);
        R_Rect_row2 = R_Rect_row2 / cv::norm(R_Rect_row2, cv::NORM_L2);

        R_Rect_row0.copyTo(R_Rect.row(0));
        R_Rect_row1.copyTo(R_Rect.row(1));
        R_Rect_row2.copyTo(R_Rect.row(2));
    }

    void Rect_Planar::cal_P_Rect(const cv::Mat& t_Orig_B, const cv::Mat& t_Orig_M)
    {
        cv::Mat Trans_Rect_B(3, 4, CV_32F);
        R_Rect.colRange(0, 3).copyTo(Trans_Rect_B.colRange(0, 3)); // StartCol is inclusive while EndCol is exclusive
        t_Orig_B.col(0).copyTo(Trans_Rect_B.col(3));

        cv::Mat Trans_Rect_M(3, 4, CV_32F);
        R_Rect.colRange(0, 3).copyTo(Trans_Rect_M.colRange(0, 3)); // StartCol is inclusive while EndCol is exclusive
        t_Orig_M.col(0).copyTo(Trans_Rect_M.col(3));

        P_Rect_B = K_Rect * Trans_Rect_B;
        P_Rect_M = K_Rect * Trans_Rect_M;
    }

    void Rect_Planar::cal_H(const cv::Mat& P_Orig_B, cv::Mat& P_Orig_M)
    {
        H_B = P_Rect_B * P_Orig_B.inv(cv::DECOMP_SVD); 
        float H_B_00 = H_B.at<float>(0, 0);
        float H_B_01 = H_B.at<float>(0, 1);
        float H_B_02 = H_B.at<float>(0, 2);
        float H_B_10 = H_B.at<float>(1, 0);
        float H_B_11 = H_B.at<float>(1, 1);
        float H_B_12 = H_B.at<float>(1, 2);
        float H_B_20 = H_B.at<float>(2, 0);
        float H_B_21 = H_B.at<float>(2, 1);
        float H_B_22 = H_B.at<float>(2, 2);
        H_M = P_Rect_M * P_Orig_M.inv(cv::DECOMP_SVD);
    }

    //---Get Function---
    void Rect_Planar::get_RectImg(cv::Mat& Output_RectImgB, cv::Mat& Output_RectImgM)
    {
        Output_RectImgB = Img_Rect_B;
        Output_RectImgM = Img_Rect_M;
    }

} // Stereo
