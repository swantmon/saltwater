#pragma once

#ifndef FutoGmtCV_H
#define FutoGmtCV_H

#include <vector>

#include "plugin\stereo\stereo_PolarRect.h"

#include "opencv2/opencv.hpp"

namespace Stereo
{
    class FutoGmtCV
    {
    //---Constructor & Destructor---
    public:
        FutoGmtCV();
        FutoGmtCV(cv::Mat& Img_Input);
        FutoGmtCV(const std::vector<char>& Img_Input, int ImgW, int ImgH);
        ~FutoGmtCV();

    //---Photogrammetric Computer Vision---
    public:
        void cal_PolarRect(cv::Mat& RectImg_Base, cv::Mat& RectImg_Match, const cv::Mat& Img_Match, const cv::Mat F_mtx);

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
        cv::Mat get_Img();
        cv::Mat get_P_mtx();

    //---Is Function---
    public:
        bool is_InsideImg(cv::Point2f ImgPt, cv::Size ImgSize);
    
    //---Export Functions---
    public:
        void show_Img(); 
    
    //---Operator Object---
    private:
        PolarRect oper_PolarRect;

    //---Members---
    private:
        cv::Mat Img; // Image data in RGB or RGBA
        cv::Mat K_mtx; // Camera Matrix in pixel. Origin is upper-left. x-axis is row-direction and y-axis is column-direction
        cv::Mat Rot_mtx; // Rotations from Mapping frame to Image frame
        cv::Mat Trans_vec; // Projection Center in Mapping frame
        cv::Mat P_mtx; // P-matrix = [Rot_mtx | -Rot_mtx*Trans_vec]
    };
} // Stereo

#endif
