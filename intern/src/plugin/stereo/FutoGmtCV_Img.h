
#pragma once

#include "opencv2/opencv.hpp"
#include "libsgm.h"

namespace FutoGmtCV
{
    class FutoImg
    {
    //---Constructor & Destructor---
    public:
        FutoImg();
        FutoImg(cv::Mat& Img_Input);
        FutoImg(cv::Mat& Img_Input, cv::Mat P);
        FutoImg(cv::Mat& Img_Input, cv::Mat K, cv::Mat R, cv::Mat PC);
        ~FutoImg();

    //---Set Functions---
    public:
        void set_Cam(cv::Mat& K);
        void set_Rot(cv::Mat& R);
        void set_PC(cv::Mat& T);
        void set_PPM(cv::Mat P_Input);

    //---Get Function---
    public:
        cv::Mat get_Img() const;
        cv::Mat get_Cam() const;
        cv::Mat get_Rot() const;
        cv::Mat get_PC() const;
        cv::Mat get_PPM() const;
        
    //---Members---
    private:
        cv::Mat m_Img; // Image data in RGB or RGBA
        cv::Mat m_Cam_mtx; // Camera Matrix in pixel. Origin is upper-left. x-axis is row-direction and y-axis is column-direction
        cv::Mat m_Rot_mtx; // Rotations from Mapping frame to Image frame
        cv::Mat m_PC_vec; // Projection Center in Mapping frame
        cv::Mat m_P_mtx; // Perspective Projection Matrix = K_mtx * [Rot_mtx | -Rot_mtx*PC_vec]

    //===OLD===

    //---Photogrammetric Computer Vision---
    public:

        //---Stereo Matching---
        void imp_cvSGBM(cv::Mat& DispImg, const cv::Mat& RectImg_Base, const cv::Mat& RectImg_Match);
    
    //---Operator Object---
    private:
        //---Stereo Matching---
        cv::Ptr<cv::StereoSGBM> operPtr_cvSGBM;


    };
} // FutoGmtCV

