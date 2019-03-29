#pragma once

#ifndef FutoGmtCV_H
#define FutoGmtCV_H

#include <vector>

#include "plugin\stereo\stereo_Rect_Polar.h"
#include "plugin\stereo\stereo_Rect_Planar.h"

#include "opencv2/opencv.hpp"

namespace Stereo
{
    class FutoGmtCV
    {
    //---Constructor & Destructor---
    public:
        FutoGmtCV();
        FutoGmtCV(cv::Mat& Img_Input);
        FutoGmtCV(cv::Mat& Img_Input, cv::Mat P);
        FutoGmtCV(cv::Mat& Img_Input, cv::Mat K, cv::Mat R, cv::Mat PC);
        ~FutoGmtCV();

    //---Photogrammetric Computer Vision---
    public:
        //---Epipolarization---
        void imp_PolarRect(cv::Mat& RectImg_Base, cv::Mat& RectImg_Match, const cv::Mat& Img_Match, const cv::Mat F_mtx);
        void imp_PlanarRect(cv::Mat& RectImg_Base, cv::Mat& RectImg_Match, cv::Mat& Orig2Rect_B_x, cv::Mat& Orig2Rect_B_y, cv::Mat& Orig2Rect_M_x, cv::Mat& Orig2Rect_M_y, const FutoGmtCV& OrigImg_Match);
        void imp_Rect_OpenCV(cv::Mat& RectImg_Base, cv::Mat& RectImg_Match, cv::Mat& Orig2Rect_B_x, cv::Mat& Orig2Rect_B_y, cv::Mat& Orig2Rect_M_x, cv::Mat& Orig2Rect_M_y, const FutoGmtCV& OrigImg_Match);

        //---Stereo Matching---
        void imp_cvSGBM(cv::Mat& DispImg, const cv::Mat& RectImg_Base, const cv::Mat& RectImg_Match);

    //---Orientation & Transformation---
    public:
        void cal_F_mtx(const cv::Mat& P_ImgM, cv::Mat& F_mtx);

    //---Set Functions---
    public:
        void set_Cam(cv::Mat& K);
        void set_Rot(cv::Mat& R);
        void set_Trans(cv::Mat& T);
        void set_P_mtx(cv::Mat P_Input);

    //---Get Function---
    public:
        cv::Mat get_Img() const;
        cv::Mat get_Cam() const;
        cv::Mat get_Rot() const;
        cv::Mat get_PC() const;
        cv::Mat get_P_mtx() const;

    //---Is Function---
    public:
        bool is_InsideImg(cv::Point2f ImgPt, cv::Size ImgSize);
    
    //---Export Functions---
    public:
        void show_Img(); 
    
    //---Operator Object---
    private:
        //---Epipolarization---
        Rect_Polar operObj_PolarRect; // Operate Polar Rectification for Epipolarization
        Rect_Planar operObj_PlanarRect; // Operate Planar Rectification for Epipolarization

        //---Stereo Matching---
        cv::Ptr<cv::StereoSGBM> operPtr_cvSGBM;

    //---Members---
    private:
        cv::Mat Img_RGB; // Image data in RGB or RGBA
        cv::Mat K_mtx; // Camera Matrix in pixel. Origin is upper-left. x-axis is row-direction and y-axis is column-direction
        cv::Mat Rot_mtx; // Rotations from Mapping frame to Image frame
        cv::Mat PC_vec; // Projection Center in Mapping frame
        cv::Mat P_mtx; // Perspective Projection Matrix = K_mtx * [Rot_mtx | -Rot_mtx*PC_vec]
    };
} // Stereo

#endif
