#pragma once

#ifndef Rect_Planar_H
#define Rect_Planar_H

#include "opencv2/opencv.hpp"

namespace Stereo
{
    class Rect_Planar
    {
    //---Constructor & Destructor---
    public:
        Rect_Planar();
        ~Rect_Planar();

    //---Generate Rectified Images-----
    public:
        void genrt_RectifiedImg(const cv::Mat& Img_Orig_B, const cv::Mat& Img_Orig_M);
        void determ_RectiedImgSize(const cv::Size& ImgSize_OrigB, const cv::Size& ImgSize_OrigM);
        
    //---Compute Orientations---
    public:
        void cal_K_Rect(const cv::Mat& K_Orig_B, const cv::Mat& K_Orig_M);
        void cal_R_Rect(const cv::Mat& R_Orig_B, const cv::Mat& t_Orig_B, const cv::Mat& t_Orig_M);
        void cal_P_Rect(const cv::Mat& t_Orig_B, const cv::Mat& t_Orig_M);
        void cal_H(const cv::Mat& P_Orig_B, cv::Mat& P_Orig_M);

    //---Get Function---
    public:
        void get_RectImg(cv::Mat& Output_RectImgB, cv::Mat& Output_RectImgM);

    //---Members---
    private:
        //---Epipolarization-----
        cv::Mat Img_Rect_B, Img_Rect_M;
        cv::Size ImgSize_Rect_B, ImgSize_Rect_M;
        cv::Mat mapB_x_Orig2Rect, mapB_y_Orig2Rect, mapM_x_Orig2Rect, mapM_y_Orig2Rect;
        cv::Mat mapB_x_Rect2Orig, mapB_y_Rect2Orig, mapM_x_Rect2Orig, mapM_y_Rect2Orig;

        //---Orientations---
        cv::Mat K_Rect; // Camera mtx for Rectified Images
        cv::Mat R_Rect; // Rotation mtx for Rectified Images
        cv::Mat P_Rect_B, P_Rect_M; // Perspective Projection mtx for Rectified Images
        cv::Mat H_B, H_M; // Homography from Original to Rectified
    };

} // Stereo

#endif
