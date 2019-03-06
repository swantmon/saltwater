#pragma once

#ifndef Fu_FotoGmtCV_H
#define Fu_FotoGmtCV_H

#include <vector>

#include "base/base_include_glm.h" // Some warnings appears when directly #include glm 

#include "opencv2/opencv.hpp"

namespace Stereo
{
    class Fu_FotoGmtCV
    {
    //---Constructor & Destructor---
    public:
        Fu_FotoGmtCV();
        Fu_FotoGmtCV(const std::vector<char>&, int ImgW, int ImgH);
        ~Fu_FotoGmtCV();
    
    //---Epipolarization: Polar Rectification-----
    public:
        cv::Mat PolarRect(Fu_FotoGmtCV Img_Match);

    private:
        void determ_CoRegion();


    //---Set Functions---
    public:
        void set_Cam(glm::mat3& K);
        void set_Rot(glm::mat3 R);
        void set_Trans(glm::vec3 T);
        void set_P(glm::mat3x4 P);
    
    //---Show Functions---
    public:
        void show_Img(); 

    //---Type Transform---
    private:
        void glm2cv(cv::Mat* cvmat, const glm::mat3& glmmat);
        void glm2cv(cv::Mat* cvmat, const glm::vec3& glmmat);
        void glm2cv(cv::Mat* cvmat, const glm::mat3x4& glmmat);

    //---Members---
    private:
        cv::Mat Img; // Original Image in RGB or RGBA
        cv::Mat Img_Rect; // Rectified Image
        cv::Mat K_mtx;
        cv::Mat Rot_mtx; // Rotations from Mapping frame to Image frame
        cv::Mat Trans_vec; // Translations in Mapping frame
        cv::Mat P_mtx; // P-matrix = [Rot_mtx | -Rot_mtx*Trans_vec]
            /*
            glm is column-major, which makes glm::mat4*3 represents real 3*4 matrix
            Verification: glm::mat3x2 * glm::mat2x3 = glm::mat2
                glm::mat3x2 A = glm::mat3x2(glm::vec2(1, 2), glm::vec2(2, 3), glm::vec2(1, 2));
                glm::mat2x3 B = glm::mat2x3(glm::vec3(0, 1, 2), glm::vec3(2, 1, 0));
                glm::mat2 Ans = A * B;
            */
    };
} // Stereo

#endif
