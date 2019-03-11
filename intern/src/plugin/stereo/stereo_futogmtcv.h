#pragma once

#ifndef FutoGmtCV_H
#define FutoGmtCV_H

#include <vector>

#include "base/base_include_glm.h" // Some warnings appears when directly #include glm 
#include "opencv2/opencv.hpp"

#include "plugin\stereo\stereo_PolarRect.h"

namespace Stereo
{
    class FutoGmtCV
    {
    //---Constructor & Destructor---
    public:
        FutoGmtCV();
        FutoGmtCV(const std::vector<char>&, int ImgW, int ImgH);
        ~FutoGmtCV();

    //---Photogrammetric Computer Vision---
    public:
        void cal_PolarRect(const cv::Mat& Img_Match, const cv::Mat F_mtx, const cv::Point2f EpiPole_B, const cv::Point2f EpiPole_M);

    //---Orientation & Transformation---
    public:
        void cal_F_mtx(const cv::Mat& P_ImgM, cv::Mat& F_mtx);
        void cal_EpiPoles(const cv::Mat& F_mtx, cv::Point2f& EpiPole_B, cv::Point2f& EpiPole_M);

    //---Set Functions---
    public:
        void set_Cam(glm::mat3& K);
        void set_Rot(glm::mat3 R);
        void set_Trans(glm::vec3 T);
        void set_P_mtx(glm::mat4x3 P);

    //---Get Function---
    public:
        cv::Mat get_Img();
        cv::Mat get_P_mtx();

    //---Is Function---
    public:
        bool is_InsideImg(cv::Point2f ImgPt, cv::Size ImgSize);
    
    //---Show Functions---
    public:
        void show_Img(); 

    //---Type Transform---
    private:
        void glm2cv(cv::Mat* cvmat, const glm::mat3& glmmat);
        void glm2cv(cv::Mat* cvmat, const glm::vec3& glmmat);
        void glm2cv(cv::Mat* cvmat, const glm::mat3x4& glmmat);
    
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
